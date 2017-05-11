#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <avr/sleep.h>  // include the Arduino (AVR) sleep functions.
#include <DHT11.h>
#include <Wire.h>
#include "RTClib.h"

//Definimos el sensor de temperatura y humedad DHT11
int pin=2;
DHT11 dht11(pin);

//Definimos el reloj RTC
RTC_DS3231 rtc;

//Definimos los dias de la semana para el RTC
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Definimos los pines y valores de tension y resistencia para medir la fotoresistencia:
const int sensorPin = A0;
const int Rc = 1000;  // valor de la resistencia de calibración
int V;   // almacena el valor medido
long fotoresistencia;   // almacena la resistencia calculada

//Definimos el semaforo que utilizaremos posteriormente para gestionar el acceso al puerto serie
SemaphoreHandle_t xSemaphore = NULL;

// define two tasks for Blink & AnalogRead
void TareaTempHum( void *pvParameters );
void TareaReloj( void *pvParameters );
void Tareafotoresistencia( void *pvParameters );

// the setup function runs once when you press reset or power the board
void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  //Comprobamos que el RTC esta funcionando
   if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
   }
   if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }

  
  // Now set up two tasks to run independently.
  xTaskCreate(
    TareaTempHum
    ,  (const portCHAR *)"TareaTempHum"   // A name just for humans
    ,  100  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TareaReloj
    ,  (const portCHAR *) "TareaReloj"
    ,  200  // Stack size
    ,  NULL
    ,  3  // Priority
    ,  NULL );

  xTaskCreate(
    Tareafotoresistencia
    ,  (const portCHAR *)"Tareafotoresistencia"   // A name just for humans
    ,  100  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );
    
  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  xSemaphore = xSemaphoreCreateMutex();
}

void loop()
{
  // Digital Input Disable on Analogue Pins
  // When this bit is written logic one, the digital input buffer on the corresponding ADC pin is disabled.
  // The corresponding PIN Register bit will always read as zero when this bit is set. When an
  // analogue signal is applied to the ADC7..0 pin and the digital input from this pin is not needed, this
  // bit should be written logic one to reduce power consumption in the digital input buffer.
   
  #if defined(__AVR_ATmega640__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega1281__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__) // Mega with 2560
  DIDR0 = 0xFF;
  DIDR2 = 0xFF;
  #elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644PA__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega1284PA__) // Goldilocks with 1284p
  DIDR0 = 0xFF;
   
  #elif defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) // assume we're using an Arduino with 328p
  DIDR0 = 0x3F;
   
  #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) // assume we're using an Arduino Leonardo with 32u4
  DIDR0 = 0xF3;
  DIDR2 = 0x3F;
  #endif
   
  // Analogue Comparator Disable
  // When the ACD bit is written logic one, the power to the Analogue Comparator is switched off.
  // This bit can be set at any time to turn off the Analogue Comparator.
  // This will reduce power consumption in Active and Idle mode.
  // When changing the ACD bit, the Analogue Comparator Interrupt must be disabled by clearing the ACIE bit in ACSR.
  // Otherwise an interrupt can occur when the ACD bit is changed.
  ACSR &= ~_BV(ACIE);
  ACSR |= _BV(ACD);
   
  // There are several macros provided in the header file to actually put
  // the device into sleep mode.
  // SLEEP_MODE_IDLE (0)
  // SLEEP_MODE_ADC (_BV(SM0))
  // SLEEP_MODE_PWR_DOWN (_BV(SM1))
  // SLEEP_MODE_PWR_SAVE (_BV(SM0) | _BV(SM1))
  // SLEEP_MODE_STANDBY (_BV(SM1) | _BV(SM2))
  // SLEEP_MODE_EXT_STANDBY (_BV(SM0) | _BV(SM1) | _BV(SM2))
   
  set_sleep_mode( SLEEP_MODE_IDLE );
   
  portENTER_CRITICAL();
  sleep_enable();
   
  // Only if there is support to disable the brown-out detection.
  #if defined(BODS) && defined(BODSE)
  sleep_bod_disable();
  #endif
   
  portEXIT_CRITICAL();
  sleep_cpu(); // good night.
   
  // Ugh. I've been woken up. Better disable sleep mode.
  sleep_reset(); // sleep_reset is faster than sleep_disable() because it clears all sleep_mode() bits.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TareaTempHum(void *pvParameters)  // This is a task.
{
  (void) pvParameters;
  int err;
  float temp, hum;

  for (;;) // A Task shall never return or exit.
  {
    if( xSemaphore != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
            if((err = dht11.read(hum, temp)) == 0){ // Si devuelve 0 es que ha leido bien
               Serial.print("T: ");
               Serial.print(temp);
               Serial.print(" H: ");
               Serial.print(hum);
               Serial.println();
            }else{
               Serial.println();
               Serial.print("Error Num :");
               Serial.print(err);
               Serial.println();
            }
            xSemaphoreGive( xSemaphore );
        }
    }
    vTaskDelay(1500/portTICK_PERIOD_MS); // wait for one second
  }
}

void TareaReloj(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    if( xSemaphore != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
           DateTime now = rtc.now();

           Serial.print(now.year(), DEC);
           Serial.print('/');
           Serial.print(now.month(), DEC);
           Serial.print('/');
           Serial.print(now.day(), DEC);
           Serial.print(" (");
           Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
           Serial.print(") ");
           Serial.print(now.hour(), DEC);
           Serial.print(':');
           Serial.print(now.minute(), DEC);
           Serial.print(':');
           Serial.print(now.second(), DEC);
           Serial.println();
           xSemaphoreGive( xSemaphore );
        }
     }
     vTaskDelay(1000/portTICK_PERIOD_MS); // wait for one second
  }
}

void Tareafotoresistencia(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    if( xSemaphore != NULL )
    {
        /* See if we can obtain the semaphore.  If the semaphore is not
        available wait 10 ticks to see if it becomes free. */
        if( xSemaphoreTake( xSemaphore, ( TickType_t ) 10 ) == pdTRUE )
        {
           V = analogRead(sensorPin); //realizar la lectura
           fotoresistencia = 1024L * Rc / V - Rc;   //calcular el valor de la resistencia
           Serial.print("Fotoresistencia: ");
           Serial.print(fotoresistencia);
           Serial.println();
           xSemaphoreGive( xSemaphore );
        }
     }
     vTaskDelay(1000/portTICK_PERIOD_MS); // wait for one second
  }
}
