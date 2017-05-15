//Configuramos los pines para el motor de DC
int pwm_a = 6;  //PWM control for motor outputs 1 and 2 
int dir_a = 7;  //direction control for motor outputs 1 and 2
const int ledPin = 13;

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
       //Definimos el sensor para las luces de la casa
       pinMode(ledPin, OUTPUT);
   }

void loop()
   {
       string mensaje;
       if(Serial.available() > 0){ 
          mensaje = Serial.read(); //almacenar byte en la variable mensaje
       }
       
       if (mensaje == "ventON"){
          digitalWrite(9,HIGH);
          digitalWrite(8,LOW);
       }else if(mensaje == "ventOFF"){
          digitalWrite(9,LOW);
          digitalWrite(8,LOW);
       }else if(mensaje == "ledOFF")){
          digitalWrite(ledPin, LOW);
       }else if(mensaje == "ledON"){
          digitalWrite(ledPin, HIGH);
       }else if(mensaje == "subePersianas"){
          motor_DC(LOW, 255);
       }else if(mensaje == "bajaPersianas"){
          motor_DC(HIGH, 255);
       }

void motor_DC(int dir, int mov)
{
  digitalWrite(dir_a, dir); 
  analogWrite(pwm_a, mov);  
  delay(1000);
  analogWrite(pwm_a, 0);  
}
