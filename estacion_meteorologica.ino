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

//Configuramos los pines para el motor de DC
int pwm_a = 6;  //PWM control for motor outputs 1 and 2 
int dir_a = 7;  //direction control for motor outputs 1 and 2

void setup()
   {
       Serial.begin(9600);
       //Los pines 8 y 9 son utilizados para controlar la dirección del ventilador.
       //La potencia con la que se mueve el ventilador depende del voltaje de entrada.
       pinMode(9,OUTPUT); 
       pinMode(8,OUTPUT);
       //Configuramos los pines del motor de DC
       pinMode(pwm_a, OUTPUT);  //Set control pins to be outputs
       pinMode(dir_a, OUTPUT); 
       analogWrite(pwm_a, 100);  //set both motors to run at (100/255 = 39)% duty cycle (slow)
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
   }

void loop()
   {
       int err;
       float temp, hum;
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
       
       if((err = dht11.read(hum, temp)) == 0){ // Si devuelve 0 es que ha leido bien
             Serial.print("Temperatura: ");
             Serial.print(temp);
             Serial.print(" Humedad: ");
             Serial.print(hum);
             Serial.println();
             //El movimiento del ventilador va en función de los valores que se escriben en el pin 9 y 8,
             // si se invierten sus valores va en un sentido o en otro.
             if(temp > 40){
              digitalWrite(9,HIGH);
              digitalWrite(8,LOW);
             }else{
              digitalWrite(9,LOW);
              digitalWrite(8,LOW);
             }  
          }else{
             Serial.println();
             Serial.print("Error Num :");
             Serial.print(err);
             Serial.println();
          }
       V = analogRead(sensorPin); //realizar la lectura
       fotoresistencia = 1024L * Rc / V - Rc;   //calcular el valor de la resistencia
       Serial.print("Fotoresistencia: ");
       Serial.print(fotoresistencia);
       Serial.println();
       //Se le pasa la direccion (H o L) y el movimiento (0 - 255)
       motor_DC(LOW, 255);
       delay(1000);
   }

void motor_DC(int dir, int mov)
{
  digitalWrite(dir_a, dir); 
  analogWrite(pwm_a, mov);  
  delay(1000);
  analogWrite(pwm_a, 0);  
}
