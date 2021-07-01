#include <Ultrasonic.h>
#include <Adafruit_TiCoServo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN       10
int irsensor1 = 4;
int irsensor2 = 8;
//define right motor control pins
#define int1 3
#define int2 5

//define left motor control pins
#define int3 6
#define int4 11

//define two arrays with a list of pins for each motor
uint8_t RightMotor[2] = {int1, int2};
uint8_t LeftMotor[2] = {int3, int4};

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      11

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

////////////////////////////////////////////// 
//        RemoteXY include library          // 
////////////////////////////////////////////// 

// RemoteXY select connection mode and include library  
#define REMOTEXY_MODE__HARDSERIAL

#include <RemoteXY.h> 

// RemoteXY connection settings  
#define REMOTEXY_SERIAL Serial 
#define REMOTEXY_SERIAL_SPEED 9600 


// RemoteXY configurate   
#pragma pack(push, 1) 
uint8_t RemoteXY_CONF[] = 
  { 255,7,0,1,0,46,0,8,24,0,
  5,52,45,9,43,43,31,209,24,3,
  3,88,35,10,26,94,14,6,0,3,
  33,25,25,24,14,1,0,36,41,12,
  12,35,33,72,0,66,0,4,4,9,
  21,77,24 }; 
   
// this structure defines all the variables of your control interface  
struct { 

    // input variable
  int8_t joystick_x; // =-100..100 x-coordinate joystick position 
  int8_t joystick_y; // =-100..100 y-coordinate joystick position 
  uint8_t slider; // =0 if select position A, =1 if position B, =2 if position C, ... 
  uint8_t rgb_r; // =0..255 Red color value 
  uint8_t rgb_g; // =0..255 Green color value 
  uint8_t rgb_b; // =0..255 Blue color value 
  uint8_t button; // =1 if button pressed, else =0 

    // output variable
  int8_t battery; // =0..100 level position 

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0 

} RemoteXY; 
#pragma pack(pop) 

///////////////////////////////////////////// 
//           END RemoteXY include          // 
///////////////////////////////////////////// 

#define SERVO_PIN    9
#define SERVO_MIN 550 // 1 ms pulse
#define SERVO_MAX 2150 // 2 ms pulse
Adafruit_TiCoServo servo;

int n=0;
long unsigned battery_volts=0;
int battery_pin = A3;
#define PIN_BUTTON 7

Ultrasonic ultrasonic(12, 13);
int distancel;
int distancer;
int distancef;
int stuck = false;

void setup()  
{ 
  RemoteXY_Init ();  
  pixels.begin();
  servo.attach(SERVO_PIN, SERVO_MIN, SERVO_MAX);
  pinMode (PIN_BUTTON, OUTPUT);
  pinMode (int1, OUTPUT);
  pinMode (int2, OUTPUT);
  pinMode (int3, OUTPUT);
  pinMode (int4, OUTPUT);
  pinMode (irsensor1,INPUT);
  pinMode (irsensor2,INPUT);
  RemoteXY.battery = map(analogRead(battery_pin), 500 , 890 ,0 , 100);
} 

void loop()  
{  
  RemoteXY_Handler ();   // use the RemoteXY structure for data transfer
  digitalWrite(PIN_BUTTON, (RemoteXY.button==0)?LOW:HIGH);
  servo.write(1350);
  
  battery_volts = analogRead(battery_pin) + battery_volts;
  n++;
  if(n==1000){
  battery_volts = battery_volts/1000;
  n=0;
  RemoteXY.battery = map(battery_volts, 500 , 890 ,0 , 100);
  battery_volts=0;
  }
  
  for(int i=0;i<NUMPIXELS;i++){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(RemoteXY.rgb_r,RemoteXY.rgb_g,RemoteXY.rgb_b)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
   
    if(RemoteXY.slider == 0){
                  if(RemoteXY.joystick_y > 0){              
  jsteering (RightMotor, RemoteXY.joystick_y + RemoteXY.joystick_x);
  jsteering (LeftMotor, RemoteXY.joystick_y - RemoteXY.joystick_x);
                  }
        else if(RemoteXY.joystick_y < 0){
   jsteering (RightMotor, RemoteXY.joystick_y - RemoteXY.joystick_x);
  jsteering (LeftMotor, RemoteXY.joystick_y + RemoteXY.joystick_x);
          }
          else{
     digitalWrite (RightMotor [0], LOW);
     digitalWrite (RightMotor [1], LOW);
     digitalWrite (LeftMotor [0], LOW);
     digitalWrite (LeftMotor [1], LOW);
            }
  }
  else if(RemoteXY.slider == 1){
   irsteering();
    }
    else if(RemoteXY.slider == 2){
       ussteering();
      }
}

void jsteering (uint8_t * motor, int v) // v = motor speed, motor = pointer to an array of pins 
{
  if (v > 100) v=100;
  if (v < -100) v=-100;
  if (v > 0){
    analogWrite (motor [0],  v * 2.55);
    digitalWrite (motor [1], LOW);
  }
  else if ( v<0 ){
    digitalWrite (motor [0], LOW);
    analogWrite (motor [1], (-v) * 2.55);
  }
  else{
    digitalWrite (motor [0], LOW);
    digitalWrite (motor [1], LOW);
    analogWrite (motor [2], 0);
  }
}

void irsteering(){
  if(digitalRead(irsensor1)==HIGH && digitalRead(irsensor2)==HIGH) 
 {
   
     digitalWrite(int2, 0);
    digitalWrite(int4, 0);
    analogWrite(int1,100);
    analogWrite(int3,100);
 } 
  else if(digitalRead(irsensor1)==HIGH) 
 {
    digitalWrite(int2, 0);
    digitalWrite(int4, 0);
    analogWrite(int1,50);
    analogWrite(int3,100);
 } 
 else if(digitalRead(irsensor2)==HIGH)
 {
  digitalWrite(int2, 0);
    digitalWrite(int4, 0);
    analogWrite(int1,100);
    analogWrite(int3,50);
 }
 else{ 
    digitalWrite(int2, 0);
    digitalWrite(int4, 0);
    digitalWrite(int1,0);
    digitalWrite(int3,0);
  }
}

void ussteering(){
   servo.write(1350);
   if(stuck == true){
      delay(500);
      stuck = false;
    } 
  distancef = ultrasonic.read();
  if (distancef < 20){
    stuck = true;
    analogWrite(int1, 0);
    analogWrite(int2, 0);
    analogWrite(int3, 0);
    analogWrite(int4, 0);
      delay(500);
      servo.write(550);
      delay(500);
      distancel = ultrasonic.read();
      servo.write(2150);
      delay(500);
      distancer = ultrasonic.read();
      if(distancel > distancer && distancel > 20){
        digitalWrite(int1, LOW);
           digitalWrite(int4, LOW);
           analogWrite(int3,200);
           analogWrite(int2,200);
             delay(50);
        }
        else if(distancer > distancel && distancer > 20){
           digitalWrite(int2, LOW);
        digitalWrite(int3, LOW); 
        analogWrite(int1,200);
        analogWrite(int4,200);
        delay(50);
          }
          else{
            digitalWrite(int2, LOW);
            digitalWrite(int3, LOW); 
            analogWrite(int1,200);
            analogWrite(int4,200);
            delay(100);
            }
  }
  else if (distancef > 20){
    analogWrite(int2, 0);
    analogWrite(int4, 0);
    analogWrite(int1,200);
    analogWrite(int3,200);
    }
  }
