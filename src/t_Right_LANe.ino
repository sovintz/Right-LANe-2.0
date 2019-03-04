//  ____  _       _     _     _        _    _   _        ____    ___  
// |  _ \(_) __ _| |__ | |_  | |      / \  | \ | | ___  |___ \  / _ \ 
// | |_) | |/ _` | '_ \| __| | |     / _ \ |  \| |/ _ \   __) || | | |
// |  _ <| | (_| | | | | |_  | |___ / ___ \| |\  |  __/  / __/ | |_| |
// |_| \_\_|\__, |_| |_|\__| |_____/_/   \_\_| \_|\___| |_____(_)___/ 
//          |___/                                                     
//Made by Lan Sovinc for Right LANe MARK 2 electric longboard

/*SERIAL PRINT SWITCH (uncomment to enable printing to the Serial Monitor)________________________________________________________________*/
#define Sprintln(a) //(Serial.println(a))
#define Sprint(b) //(Serial.print(b))

/*LIBRARIES_______________________________________________________________________________________________________________________________*/
#include <SPI.h>
#include "RF24.h"

/*VARIABLES_______________________________________________________________________________________________________________________________*/
// Buttons, joystick and LEDs
int joystick = A0;
int cButton = 5;
int zButton = 2;
int red = 3;
int orange = 4;
int yellow = 6;
int green = 9;
int notConnected = 10;
volatile bool zButtonReleased = true;
bool cButtonState = 0;
bool cButtonStatePrevious = 0;
int neutral = 512;
int currentThrottle = neutral;

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

// Timer-related
unsigned long previousMillis = 0;
const long interval = 20;

/*SETUP FUNCTION__________________________________________________________________________________________________________________________*/
void setup() {

  Serial.begin(115200);
  Sprintln("Setting up");

  // IO pins
  pinMode(cButton, INPUT);
  pinMode(zButton, INPUT);
  
  // Radio initialization
  radio.begin();
  radio.enableAckPayload(); // Allow optional ack payloads
  radio.enableDynamicPayloads();  // Ack payloads are dynamic payloads
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(address1);
  radio.stopListening();

  // Interrupt setup
  attachInterrupt(digitalPinToInterrupt(2), setNeutral, FALLING);

  // LEDs
  pinMode(red, OUTPUT);
  pinMode(orange, OUTPUT);
  pinMode(yellow, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(notConnected, OUTPUT);

  // Set notConnected-LED
  digitalWrite(notConnected, HIGH);

  // Default buttons and joystick values
  data.joystickValue = neutral;
  data.frontLight = false;
  data.backLight = false;

}

/*LOOP FUNCTION___________________________________________________________________________________________________________________________*/
void loop() {

  unsigned long currentMillis = millis();
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    // Read remote control input buttons and joystick
    readThrottle();
    readLightButton();
      
    // Transmit the data struct to the other radio
    if (radio.write(&data,sizeof(data))){
  
      Sprint("Sent joystick value: ");
      Sprintln(data.joystickValue); 
      Sprint("Sent front light: ");
      Sprintln(data.frontLight); 
      Sprint("Sent back light: ");
      Sprintln(data.backLight);
      Sprintln();
      
      // Recieve ACK with skate battery level      
      while(radio.available() ){
        // Read ACK and save skateboard battery voltage
        radio.read( &skateBattery, 1 );
        // Display battery voltage on the remote
        readBattery();
        Sprint("Received skateboard battery level: ");
        Sprintln(skateBattery);
        Sprintln();
        
        // Reset notConnected-LED
        digitalWrite(notConnected, LOW);
      }  
    }
    // If no ACK response, sending failed
    else{
      // Reset battery indicator LEDs and set notConnected-LED
      digitalWrite(red, LOW);
      digitalWrite(orange, LOW);
      digitalWrite(yellow, LOW);
      digitalWrite(green, LOW);
      digitalWrite(notConnected, HIGH);
      Sprintln("Sending failed.");
    }
  }
}

/*READ THROTTLE JOYSTICK FUNCTION_________________________________________________________________________________________________________*/
void readThrottle(){
  // If safety button is pressed read the throttle value
  if(digitalRead(zButton) == HIGH){
    currentThrottle = analogRead(joystick);
    if(!zButtonReleased || (currentThrottle > 512 && currentThrottle < 530)){ // the statement forces the user to press the safety button before pushing the throttle joystick
      zButtonReleased = false;
      data.joystickValue = analogRead(joystick);
    }
  }
}

/*RELEASED SAFETY BUTTON (Z) INTERRUPT FUNCTION___________________________________________________________________________________________*/
void setNeutral(){
  data.joystickValue = neutral;
  zButtonReleased = true;
  
}

/*READ LIGHT CONTROL BUTTON (C) FUNCTION__________________________________________________________________________________________________*/
void readLightButton(){
  cButtonState = digitalRead(cButton);
  if(cButtonState != cButtonStatePrevious){
    if(cButtonState == HIGH){
      if((!data.frontLight)&&(!data.backLight)){
        data.backLight = !data.backLight;  
      }
      else if((!data.frontLight)&&(data.backLight)){
        data.frontLight = !data.frontLight;  
      }
      else{
        data.backLight = !data.backLight;
        data.frontLight = !data.frontLight;
      }
    }
  }
  cButtonStatePrevious = cButtonState;
}

/*READ SKATEBOARD BATTERY VOLTAGE FUNCTION________________________________________________________________________________________________*/
void readBattery(){

  // More than 40V
  if (skateBattery > 925){
    digitalWrite(red, LOW);
    digitalWrite(orange, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, HIGH);
  }
  // More than 37V
  else if (skateBattery > 855){
    digitalWrite(red, LOW);
    digitalWrite(orange, LOW);
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);
  }
  // More than 35V
  else if (skateBattery > 805){
    digitalWrite(red, LOW);
    digitalWrite(orange, HIGH);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
  }
  else{
    digitalWrite(red, HIGH);
    digitalWrite(orange, LOW);
    digitalWrite(yellow, LOW);
    digitalWrite(green, LOW);
  }
}
