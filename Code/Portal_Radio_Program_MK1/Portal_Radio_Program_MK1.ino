/**
 * Program to control individual components of the Portal Radio
 * Commands are received via Serial over USB and excecuted on this micro controller
 * Created:  21.07.2021
 * Modified: 25.07.2021
 * Author: Philipp Schulz
 */
 
 // library imports
#include <ESP8266WiFi.h>

// pins for all LEDs / Transistors / analog pins
#define GREEN_LED 5
#define BLUE_LED_TEXT 2
#define BLUE_LED_RING_1 0
#define BLUE_LED_RING_2 14
#define BLUE_LED_RING_3 12
#define BLUE_LED_RING_4 13
#define TRANSISTOR_VOLUME_NEG 15
#define TRANSISTOR_POWER 10
#define TRANSISTOR_VOLUME_POS 4
#define ANALOG_INPUT A0

// variables to track states of all components
bool greenLEDState = 0;
bool textLEDState = 0;
bool ringLED1State = 0;
bool ringLED2State = 0;
bool ringLED3State = 0;
bool ringLED4State = 0;
bool volumeDownState = 0;
bool volumeUpState = 0;
bool powerState = 0;
bool speakerPowerState= 0;
bool speakerConnectedState = 0;
double volume = 0 ;                                       // volume of the speaker in percent
double volumeStepSize = 6.667;                            // step size of one button press

// method to "turn radio on or off", meaning just toggling two of the LEDs and then toggling the speaker
// newState: new state of the radio, should it be on or off?
void turnRadioOnOrOff(bool newState) {
  if(greenLEDState != newState) {                         // check if green LED needs to be toggled
    greenLEDState = newState;                             // set new green LED state
    digitalWrite(GREEN_LED, greenLEDState);               // write new state to green LED
  }
  if(textLEDState != newState) {                          // check if text LED needs to be toggled
    textLEDState = newState;                              // set new text LED state
    digitalWrite(BLUE_LED_TEXT, textLEDState);            // write new state to text LED
  }
  turnSpeakerOnOrOff(newState);                           // toggle speaker
  Serial.println("OK/#/");                                // send end signal
}

// method to turn speaker on or off
// newState: new state of the speaker, should it be on of off?
void turnSpeakerOnOrOff(bool newState) {
  if(speakerPowerState != newState) {                     // check if speaker needs to be toggled
    if(newState) {                                        // if it needs to be turned on
      // turn on speaker
      digitalWrite(TRANSISTOR_POWER, 1);                  // enable power transistor
      delay(2000);                                        // wait for 2 seconds
      digitalWrite(TRANSISTOR_POWER, 0);                  // diable power transistor
      delay(3000);                                        // wait additional 3 seconds for speaker to initialize
      speakerPowerState = 1;                              // set speaker power state
      // connect speaker to adio cable device
      digitalWrite(TRANSISTOR_POWER, 1);                  // enable power transistor
      delay(200);                                         // wait for 200 ms
      digitalWrite(TRANSISTOR_POWER, 0);                  // disable power transistor to complete first button press
      delay(200);                                         // wait for 200 ms
      digitalWrite(TRANSISTOR_POWER, 1);                  // enable power transistor
      delay(200);                                         // wait for 200 ms
      digitalWrite(TRANSISTOR_POWER, 0);                  // disable power transistor to complete second button press
      delay(1000);                                        // wait for connection to establish
      speakerConnectedState = 1;                          // set speaker connected state
      // set new states of ring LEDs to indicate the speaker connection
      ringLED1State = true;                               // set new state of LED1 variable
      digitalWrite(BLUE_LED_RING_1, ringLED1State);       // turn LED1 on
      delay(100);                                         // wait 100 ms
      ringLED2State = true;                               // set new state of LED2 variable
      digitalWrite(BLUE_LED_RING_2, ringLED2State);       // turn LED2 on
      delay(100);                                         // wait 100 ms
      ringLED3State = true;                               // set new state of LED3 variable
      digitalWrite(BLUE_LED_RING_3, ringLED3State);       // turn LED3 on
      delay(100);                                         // wait 100 ms
      ringLED4State = true;                               // set new state of LED4 variable
      digitalWrite(BLUE_LED_RING_4, ringLED4State);       // turn LED4 on
    } else {                                              // if it needs to be turned off
      // turn off speaker
      digitalWrite(TRANSISTOR_POWER, 1);                  // enable power transistor
      delay(2000);                                        // wait for 2 seconds
      digitalWrite(TRANSISTOR_POWER, 0);                  // diable power transistor
      delay(2000);                                        // wait additional 2 seconds for speaker to shut down
      speakerPowerState = 0;                              // set speaker power state
      speakerConnectedState = 0;                          // set speaker connected state
      // turn off ring LEDs
      ringLED1State = false;                              // set new state of LED1 variable
      digitalWrite(BLUE_LED_RING_1, ringLED1State);       // turn LED1 off
      delay(100);                                         // wait 100 ms
      ringLED2State = false;                              // set new state of LED2 variable
      digitalWrite(BLUE_LED_RING_2, ringLED2State);       // turn LED2 off
      delay(100);                                         // wait 100 ms
      ringLED3State = false;                              // set new state of LED3 variable
      digitalWrite(BLUE_LED_RING_3, ringLED3State);       // turn LED3 off
      delay(100);                                         // wait 100 ms
      ringLED4State = false;                              // set new state of LED4 variable
      digitalWrite(BLUE_LED_RING_4, ringLED4State);       // turn LED4 off
    }
  }
}

// method to determine state of speaker power
bool determineSpeakerPowerState() {
  double speakerValue = 0;                                // initialize container for analog readings
  for(int i=0; i<300;i++) {                               // take multiple measurements to accound for pulses of source
    speakerValue += analogRead(ANALOG_INPUT);             // add analog measurement to container
  }
  return (speakerValue/300) < 100;                        // return whether the speaker is on or off
}

// method to set new volume
// percentage: new percentage of volume
void setNewVolume(double percentage) {
  if(percentage > volume) {                             // if a higher volume is requested
    while(percentage >= volume) {                       // loop until target is reached
      digitalWrite(TRANSISTOR_VOLUME_POS,1);            // enable volume down transistor
      delay(200);                                       // wait 200 ms to simulate button press
      digitalWrite(TRANSISTOR_VOLUME_POS,0);            // disable volume down transistor
      delay(200);                                       // wait for speaker to lower volume
      volume += volumeStepSize;                         // increase current volume
    }
  } else {                                              // if a lower volume is requested
    while(percentage <= volume) {                       // loop until target is reached
      digitalWrite(TRANSISTOR_VOLUME_NEG,1);            // enable volume down transistor
      delay(200);                                       // wait 200 ms to simulate button press
      digitalWrite(TRANSISTOR_VOLUME_NEG,0);            // disable volume down transistor
      delay(200);                                       // wait for speaker to lower volume
      volume -= volumeStepSize;                         // increase current volume
    }
  }
  Serial.println("OK/#/");                                // send end signal
}

// method to bring the speaker volume to the lowest possible setting
void homeVolume() {
  for(int i = 0; i < 15; i++) {                           // loop for 15 times
    digitalWrite(TRANSISTOR_VOLUME_NEG,1);                // enable volume down transistor
    delay(200);                                           // wait 200 ms to simulate button press
    digitalWrite(TRANSISTOR_VOLUME_NEG,0);                // disable volume down transistor
    delay(500);                                           // wait for speaker to lower volume
  }
  volume = 0;                                             // set the internal variable to 0
  Serial.println("OK/#/");                                // send end signal
}

// method to print current status of all components
void getComponentStatus() {
  Serial.println("Green LED: "+String(greenLEDState));
  Serial.println("Text LED: "+String(textLEDState));
  Serial.println("Ring 1 LED: "+String(ringLED1State));
  Serial.println("Ring 2 LED: "+String(ringLED2State));
  Serial.println("Ring 3 LED: "+String(ringLED3State));
  Serial.println("Ring 4 LED: "+String(ringLED4State));
  Serial.println("Volume Down Transistor: "+String(volumeDownState));
  Serial.println("Volume Up Transistor: "+String(volumeUpState));
  Serial.println("Speaker Power Transistor: "+String(powerState));
  Serial.println("Speaker Power: "+String(speakerPowerState));
  Serial.println("Speaker Connected: "+String(speakerConnectedState));
  Serial.println("Speaker Volume: "+String(volume));
  Serial.println("OK/#/");
}

// method to send back init signal
void sendInitSignal() {
  Serial.println("RADIO");
  Serial.println("OK/#/");
}

// method to handle incoming messages
void handleSerialInput(String input) {
  if(input.indexOf("INIT") >= 0) {                        // if the init signal was sent
    sendInitSignal();                                     // call method to handle init signal
  } else if(input.indexOf("STATUS") >= 0) {               // if the status signal was sent
    getComponentStatus();                                 // call method to handle status signal
  } else if(input.indexOf("RADIO") >= 0) {                // if the radio command was sent
    bool newState = false;                                // initialize new state variable
    if(input.indexOf("ON") >= 0) {                        // check if the on signal was present
      newState = true;                                    // set the variable
    }
    turnRadioOnOrOff(newState);                           // handle the new state
  } else if(input.indexOf("VOLUME") >= 0) {               // if the volume needs to be checked
    if(input.indexOf("HOME") >= 0) {                      // if the volume should be homed
      homeVolume();                                       // home the speaker
    } else {                                              // if the volume should be changed
      setNewVolume(input.substring(6).toInt());           // set the new volume
    }
  }
}

void setup() {
  WiFi.forceSleepBegin();                                 // WIFI is not required, turn it off to save power
  Serial.begin(9600);                                     // Serial for USB communication
  // initialize digital pins for LEDs and Transistors as outputs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED_TEXT, OUTPUT);
  pinMode(BLUE_LED_RING_1, OUTPUT);
  pinMode(BLUE_LED_RING_2, OUTPUT);
  pinMode(BLUE_LED_RING_3, OUTPUT);
  pinMode(BLUE_LED_RING_4, OUTPUT);
  pinMode(TRANSISTOR_VOLUME_NEG, OUTPUT);
  pinMode(TRANSISTOR_POWER, OUTPUT);
  pinMode(TRANSISTOR_VOLUME_POS, OUTPUT);
  // set initial state of all pins
  digitalWrite(TRANSISTOR_POWER,volumeDownState);
  digitalWrite(GREEN_LED, greenLEDState);
  digitalWrite(BLUE_LED_TEXT, textLEDState);
  digitalWrite(BLUE_LED_RING_1, ringLED1State);
  digitalWrite(BLUE_LED_RING_2, ringLED2State);
  digitalWrite(BLUE_LED_RING_3, ringLED3State);
  digitalWrite(BLUE_LED_RING_4, ringLED4State);
  digitalWrite(TRANSISTOR_VOLUME_NEG,volumeDownState);
  digitalWrite(TRANSISTOR_VOLUME_POS,volumeUpState);
  speakerPowerState = determineSpeakerPowerState();       // find out if the speaker is enabled or not
}

void loop() {
  if(Serial.available()) {                                // check if Serial data is available
    String input = Serial.readString();                   // read Serial data
    handleSerialInput(input);                             // handle input
  }
}
