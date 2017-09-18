
int lightSensor() {
  pixelsClear(); //turn off LEDs before reading ambient light
  return analogRead(A5); //light sensor pin A5
}


void checkButtons() {
//this should ideally be a series of case statements to make it easier to navigate menus and modes
//check if one or both buttons are pressed

  if(leftButton()) { delay(35); if(rightButton()) { mode = 1; pixelsClear(); ledPulse(9,5,255,255,0,4); } //both
                                else { 
                                  if(mode != 0) { 
                                    mode = 0; pixelsClear(); ledPulse(0,5,0,0,255,4); pixelsClear();  //Biodata Mode
                                  } else thresholdDown(); //only left button
                                }
  }
  else if(rightButton()) { delay(35); if(leftButton()) { mode = 1; pixelsClear(); ledPulse(9,5,255,255,0,4); } //both
                                      else { 
                                        if(mode != 0) { 
                                           mode = 0; pixelsClear(); ledPulse(0,5,0,0,255,4); pixelsClear();  //Biodata Mode
                                        } else thresholdUp();  //only right button
                                      }
  }


  if(slideSwitch()) { usbMIDI = 1; serialBiodata = 0; }
  else { usbMIDI = 0; serialBiodata = 1; }

  
}
  
int leftButton(){
  return digitalRead(4);
}

int rightButton(){
  return digitalRead(19);
}

int slideSwitch(){
  return digitalRead(21);
}

void ledSetup() {
//initialize the NeoPixels
  pixels.begin();
  for(byte i=0;i<LED_NUM;i++) {
    pixels.setPixelColor(i,pixels.Color(0,8,0));
    pixels.show();
    delay(75);
  }
  
  for(byte i=0;i<nodeID;i++){
  ledPulse(i, 3, 55, 0, 0, 2);
  } 
  delay(250);
  pixelsClear(); //turn off all pixels

}

void setColor( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint16_t brightness) {
  pixels.setPixelColor(n, (brightness*r/255) , (brightness*g/255), (brightness*b/255));
  pixels.show();
}


void pixelsClear() {
   for(byte i=0;i<LED_NUM;i++)  pixels.setPixelColor(i,pixels.Color(0,0,0)); //all off
    pixels.show();
}

void checkTone() {
    if(noteArray[0].velocity > 0) { //note is playing in the first slot
      double frequency = mtof(noteArray[1].value); //need to convert MIDI to frequency?
      long delayAmount = (long)(1000000/frequency);
      if(toneMicros + delayAmount <= currentMicros) { //toggle has expired
        toneToggle = !toneToggle; //swap toggle
        digitalWrite(speakerPin, toneToggle);
        toneMicros = currentMicros;
      }
    } else { if(toneToggle) { toneToggle = false; digitalWrite(speakerPin, toneToggle); } } //turn off
  
}


float temperature() {
   // Thermistor test
  float reading;
  reading = analogRead(A0); //thermistor on A0
  //Serial.print("Thermistor reading: "); Serial.println(reading);
  int SERIESRESISTOR = 10000;
  int THERMISTORNOMINAL = 10000;
  int TEMPERATURENOMINAL = 25;
  int BCOEFFICIENT = 3380;
  // convert the value to resistance
  reading = ((1023.0 * SERIESRESISTOR) / reading);
  reading -= SERIESRESISTOR;
  //Serial.print("Thermistor resistance: "); Serial.println(reading);
  float steinhart;
  steinhart = reading / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  return steinhart;
}


void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush();
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush();
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush();
}

void setNote(MIDImessage midiMessage){
  for(byte i=0;i<polyphony;i++) { //iterate through notes
    if(noteArray[i].velocity == 0) { //find a note which is not currently playing
      noteArray[i].type = 0; //note type
      noteArray[i].channel = midiMessage.channel;
      noteArray[i].value = midiMessage.value;
      noteArray[i].velocity = midiMessage.velocity;
      noteArray[i].duration = midiMessage.duration;
      noteArray[i].timeStamp = midiMessage.timeStamp;

      //setLEDdisplay(i, color); //indicate the note using the LEDs
      setColor(i,random(0,255),random(0,255),random(0,255), maxBright);

      //only send usb midi data if enabled
      if(usbMIDI)  noteOn(midiMessage.channel-1,midiMessage.value,midiMessage.velocity);
      
      break; //end for loop
    }
  }

  
}

void checkNote(){
  for(byte i=0;i<polyphony;i++) {
    if(noteArray[i].velocity > 0) { //if a note is playing
      if(noteArray[i].duration + noteArray[i].timeStamp <= currentMillis) { //note expired
      
       noteArray[i].velocity = 0;
       //setLEDdisplay(); //turn off led indication
       setColor(i,0,0,0,0); //this might work for now, still would be a better thing to fade

      //only send usb midi data if enabled
       if(usbMIDI)  noteOff(noteArray[i].channel-1,noteArray[i].value,0); //turn off note
       
      }
    }
  }

  
}


//mtof and ftom from littlescale -   Created by Sebastian Tomczak, 25 March 2017.

double base_frequency = 440.0;
double base_pitch = 69.0;

double mtof(double incoming_note) {
  return base_frequency * pow (2.0, (incoming_note - base_pitch) / 12.0);
}

double ftom(double incoming_frequency) {
  return base_pitch + (12.0 * log(incoming_frequency / base_frequency) / log(2));
}

