#include <DHT11.h>
#include <Wire.h>
#include "RTClib.h"
#include "SoftwareSerial.h"


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

void setup()
   {
       //XBee.begin(9600);
       Serial.begin(9600);
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
       String mensaje;
       DateTime now = rtc.now();

       if(Serial.available() > 0){
          mensaje = Serial.readString();
          if(mensaje[1] == '1'){
            switch(mensaje[2]){
              case '0':
                if((err = dht11.read(hum, temp)) == 0){ // Si devuelve 0 es que ha leido bien
                  if(mensaje[3] == '0'){
                    Serial.println(hum);
                  }else if(mensaje[3] == '1'){
                    Serial.println(temp);
                  }
                }
                break;
              case '1':
                switch(mensaje[3]){
                  case '0':
                    Serial.println(now.second(), DEC);
                    break;
                  case '1':
                    Serial.println(now.minute(), DEC);
                    break;
                  case '2':
                    Serial.println(now.hour(), DEC);
                    break;
                  case '3':
                    Serial.println(now.day(), DEC);
                    break;
                  case '4':
                    Serial.println(now.month(), DEC);
                    break;
                  case '5':
                    Serial.println(now.year(), DEC);
                    break;
                  case '6':
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
                     break;
                }
                break;

              case '2':
               V = analogRead(sensorPin); //realizar la lectura
               fotoresistencia = 1024L * Rc / V - Rc;   //calcular el valor de la resistencia
               Serial.println(fotoresistencia);
               break;
            }
          }
       }
   }
