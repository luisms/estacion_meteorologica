#include <DHT11.h>
#include <Wire.h>
#include "RTClib.h"
#include "SoftwareSerial.h"

SoftwareSerial XBee(3, 4);
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
       XBee.begin(9600);
       //Comprobamos que el RTC esta funcionando
       if (! rtc.begin()) {
        XBee.println("Couldn't find RTC");
        while (1);
       }
       if (rtc.lostPower()) {
        XBee.println("RTC lost power, lets set the time!");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
       }
   }

void loop()
   {
       int err;
       float temp, hum;
       DateTime now = rtc.now();

       if((err = dht11.read(hum, temp)) == 0){ // Si devuelve 0 es que ha leido bien

             XBee.print("Temperatura: ");
             XBee.print(temp);
             XBee.print(" Humedad: ");
             XBee.print(hum);
             XBee.println();

             //El movimiento del ventilador va en función de los valores que se escriben en el pin 9 y 8,
             // si se invierten sus valores va en un sentido o en otro.
             if(temp > 25){
              XBee.write('A');
             }else{
              XBee.write('B');
             }  
   /*    }else{  
             XBee.println();
             XBee.print("Error Num :");
             XBee.print(err);
             XBee.println();
             */
       
       }
       V = analogRead(sensorPin); //realizar la lectura
       fotoresistencia = 1024L * Rc / V - Rc;   //calcular el valor de la resistencia
       //XBee.print("Fotoresistencia: ");
       //XBee.print(fotoresistencia);
       //XBee.println();
       if(fotoresistencia < 15000){
         XBee.write('C');
         XBee.write('D');
       }else{
         XBee.write('E');
         XBee.write('F');
       }
       delay(1000);
   }
