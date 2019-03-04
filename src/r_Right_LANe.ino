//  ____  _       _     _     _        _    _   _        ____    ___  
// |  _ \(_) __ _| |__ | |_  | |      / \  | \ | | ___  |___ \  / _ \ 
// | |_) | |/ _` | '_ \| __| | |     / _ \ |  \| |/ _ \   __) || | | |
// |  _ <| | (_| | | | | |_  | |___ / ___ \| |\  |  __/  / __/ | |_| |
// |_| \_\_|\__, |_| |_|\__| |_____/_/   \_\_| \_|\___| |_____(_)___/ 
//          |___/                                                     
//Made by Lan Sovinc for Right LANe MARK 2 electric longboard

/*SERIAL PRINT SWITCH_____________________________________________________________________________________________________________________*/
#define Sprintln(a) //(Serial.println(a))
#define Sprint(b) //(Serial.print(b))

/*LIBRARIES_______________________________________________________________________________________________________________________________*/
#include <SPI.h>
#include <RF24.h>
#include <PWM.h>

// RF24-related
RF24 radio(7,8);
const byte address1[6] = "00001";

// Data structure for transmission
struct Data{
  int joystickValue;
  bool frontLight;
  bool backLight;
}data;

// Battery level of the skateboard
int skateBattery = 0;

// PWM-related
int32_t frequency = 50;
int pwmOut = 10;
int neutral = 4915;
long joystickValuePWM = neutral;

// Lights-related
int brightnessFront = 255;
int brightnessBack = 128;
int brightnessBreak = 191;

// Timeout-related
unsigned long previousTime = 0;
const long timeout = 500;

/*SETUP FUNCTION__________________________________________________________________________________________________________________________*/
void setup() {

  Serial.begin(115200);
  Sprintln("Setting up");

  // IO pins
  pinMode(pwmOut, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);

  // Radio initialization
  radio.begin();
  radio.enableAckPayload(); // Allow optional ack payloads
  radio.enableDynamicPayloads();  // Ack payloads are dynamic payloads
  radio.setPALevel(RF24_PA_MAX);
  radio.openReadingPipe(1,address1);
  radio.startListening();

  // PWM initialization
  InitTimersSafe();
  bool success = SetPinFrequencySafe(pwmOut, frequency);
  if (success){
    Sprintln("Custom PWM library initialized");
    Sprint("PWM frequency = ");
    Sprintln(Timer1_GetFrequency());
    Sprint("resolution: ");
  //Serial.println(Timer1_GetTop());
  }

  // Set PWM so the board won't move
  pwmWriteHR(pwmOut, neutral);

}

/*LOOP FUNCTION___________________________________________________________________________________________________________________________*/
void loop() {
  byte pipeNo;

  // Read all available payloads
  if( radio.available(&pipeNo)){
    //Sprint("PipeNo is: ");
    //Sprintln(pipeNo);
    radio.read( &data, sizeof(data) );

    // Check if the commands recived are correct
    if(((data.joystickValue>=0)&&(data.joystickValue<=1023))&&((data.backLight==0)||(data.backLight==1))&&((data.backLight==0)||(data.backLight==1))){
      //Execute the commands recived-----
      
      // Set PWM according to the joystick value
      joystickValuePWM = map(data.joystickValue, 0, 1023, 3277, 6554);
      pwmWriteHR(pwmOut, joystickValuePWM);

      // Turn on break/back/front light
      lightsControl();

      Sprint("Recieved joystick value: ");
      Sprintln(joystickValuePWM); 
      Sprint("Recieved front light: ");
      Sprintln(data.frontLight); 
      Sprint("Recieved back light: ");
      Sprintln(data.backLight);

      previousTime = millis();

      //-----
    }
    else{      
      Sprint("Incorrect data");
    }
  
    // Read skateboard battery voltage and send it as ACK
    skateBattery = analogRead(A3);
    radio.writeAckPayload(pipeNo,&skateBattery, 1 );
    Sprint("Sending battery voltage level ");
    Sprintln(skateBattery);
    Sprintln();
  }
  else{
    // If connection is lost, stop the skateboard and turn off the lights
    if (millis() - previousTime >= timeout) {
      Sprintln("Transmission failed. Resetting parameters...");
      pwmWriteHR(pwmOut, neutral);
      analogWrite(3, 0);
      analogWrite(5, 0);
      radio.stopListening();
      delay(500);
      radio.startListening();
      previousTime = millis();
    }
  }
}

/*LIGHTS CONTROL FUNCTION_________________________________________________________________________________________________________________*/
void lightsControl(){
  
  if(joystickValuePWM < neutral){
      analogWrite(5, brightnessBreak);
  }
  else{
    // Toggle back light
    analogWrite(5, data.backLight ? brightnessBack : 0);  
  }
  // Toggle front light
  analogWrite(3, data.frontLight ? brightnessFront : 0);
}

//TODO:
//pravi pipe/channel

