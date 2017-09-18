/*
 * 1. Read pulse widths from 555 timer, store in array
 * 2. Calculate 'change detection' and note values
 * 3. MIDI output - USB, Serial
 * 4. LED Light Show
 * 5. WIFI Data
 * 6. Modes and Menus
 * 7. Storing parameter settings
 * 
 * 
 */




//#include <Adafruit_CircuitPlayground.h> //super excited!! but can't use it lest modified for wifi
#include <FreqPeriodCounter.h>
#include <SoftwareSerial.h>
#include <ESP8266wifi.h>
#include <Adafruit_NeoPixel.h>
#include <MIDIUSB.h>


#define esp8266_reset_pin 2 // Connect this pin to CH_PD on the esp8266, not reset. (let reset be unconnected)

#define SERVER_IP  "192.168.1.157"   //3"  //"10.0.1.2" //
#define SERVER_PORT "1234"
#define SSID "OxfordEast" ////"touch" //
#define PASSWORD "oxfordeast" ////"password123" //

SoftwareSerial ESPSerial(10,9); //talk to the ESP 

//*******************
int nodeID = 2; //Identification of node

//*******************

//Tone note;
int speakerPin = 5;
byte speakerSound = 0; //toggle onboard speaker music
bool toneToggle = false; //speaker on off toggle
long toneMicros = 0;
long currentMicros = 0; //keep time for speaker



int serverConnected = 0; //wifi status toggle
int usbMIDI = 0; // toggle MIDI through USB
int serialBiodata = 1; //output serial data for arduino logger
byte debugSerial=0; //toggle serial terminal debugging

byte inputPin = 3; ///555 timer input, Pin 3 - SCL INT0
int reading = 0;
int prevReading = 0;

const int deltaCount = 10;
int deltaAverage[deltaCount]; //sample to identify average delta fluctutation
int deltaAvg = 0;
float threshold = 0.1; 
float thresholdMin = 0.03;
float thresholdMax = 0.85;
float thresholdAmount = 0.013;
int deltaIndex = 0;


const int LED_NUM = 10;
int ledPin = 17; // first neopixel on Circuit Playground
//manage our own neopixels, until i can get the playground.begin() to work with wifi
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_NUM, ledPin, NEO_GRB + NEO_KHZ800);
int ledPixels[LED_NUM] = {0,1,2,3,4,5,6,7,8,9};
int maxBright = 45; //brightness level to apply between 0 and 255
int fadeRate = 10; //fade for pulse?


int potPin = 11; //pin 12 in test
float tempValue = 0;
int lightValue = 0; 
int change = 0; 
int wifi_started = 0;

long currentMillis = 0;
long prevMillis = 0;


FreqPeriodCounter freqPeriodCounter(inputPin, micros, 0);
//ESP8266wifi wifi(ESPSerial, ESPSerial, esp8266_reset_pin,Serial);
ESP8266wifi wifi(ESPSerial, ESPSerial, esp8266_reset_pin); //no serial echo

const int sampleSize = 100; //number of value 
int sampleIndex = 0;
int pulseInput[sampleSize];

//menu modes navigated by buttons and LEDs
int mode = 0; //0 normal, 1 wifi signal meter

String inputString;
byte stringComplete=0;

const int polyphony = 10; //max notes to be managed locally at same time (usb MIDI, light show)
//additional notes will be ignored (unlimited poly may be sent via wifi)

typedef struct _MIDImessage { //build structure for Note and Control MIDImessages
  unsigned int type;
  int channel;
  int value;
  int velocity;
  long duration;
  long timeStamp;
} 
MIDImessage;
MIDImessage noteArray[polyphony]; //manage MIDImessage data as an array with size polyphony


void ledPulse(int led = 0, int pulses = 0, int r = 0, int g =0 , int b = 0, long duration = 0) {
  //fade specified LED the specified number of times over the duration in the color
  for(byte k=0;k<pulses;k++){
    for(byte i=0;i<maxBright;i++){ //fade, perhaps?
      setColor(led,r,g,b,i);
      delay(duration); //wait for next cycle
    }
  }
}



void setup() {

  randomSeed(A0); 
  
  Serial.begin(9600); //usb debugging
    
  ESPSerial.begin(9600); //wifi serial

  ledSetup(); //test and setup the LEDs

  wifiSetup(); //initialize wifi settings and connect to server

  pinMode(4, INPUT_PULLUP); //left button
  pinMode(19, INPUT_PULLUP); //right button
  pinMode(21, INPUT_PULLUP); //slide switch
  
  pinMode(speakerPin, OUTPUT); //speaker

  //initialize noteArray objects
  for(byte i=0;i<polyphony;i++) { //iterate through notes
      noteArray[i].type = 0; //note type
      noteArray[i].channel = 0;
      noteArray[i].value = 0;
      noteArray[i].velocity = 0;
      noteArray[i].duration = 0;
      noteArray[i].timeStamp = 0;
   }
  
  
}

void loop() {

  currentMillis = millis();
//    currentMillis = currentMicros/1000;
  currentMicros = micros();

  checkButtons();
  
  if(mode == 0) checkInput(); //normal running mode
  if(mode == 1) { //wifi signal meter
    if(currentMillis - prevMillis > 5000){
      prevMillis = currentMillis;  
      //environmentSensors();
      measureSignal(); //wifi signal meter
    }
  }
  
  checkNote(); //update notes, turn on or off accordingly
  if(speakerSound) checkTone();
  

}




