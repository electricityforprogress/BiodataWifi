//provide float map function
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void analyzeSample() {

  unsigned long averg = 0;
  unsigned long maxim = 0;
  unsigned long minim = 100000;
  float stdevi = 0;
  unsigned long delta = 0;
  byte change = 0;

  if (sampleIndex == sampleSize) { //array is full //horrible bug fixed 20170827 ==
    unsigned long sampanalysis[sampleSize];
    for (byte i=0; i<sampleSize; i++){ 
      sampanalysis[i] = pulseInput[i];  //load analysis table (due to volitle)
      //manual calculation
      if(sampanalysis[i] > maxim) { maxim = sampanalysis[i]; }
      if(sampanalysis[i] < minim) { minim = sampanalysis[i]; }
      averg += sampanalysis[i];
      stdevi += sampanalysis[i] * sampanalysis[i];  //prep stdevi
    }

    //manual calculation
    averg = averg/sampleSize;
    stdevi = sqrt(stdevi / sampleSize - averg * averg); //calculate stdevu
    if (stdevi < 1) { stdevi = 1.0; } //min stdevi of 1
    delta = maxim - minim; 

  // threshold is a measurement above or below a set of sample averages
  // riding along with the average delta, threshold representsan amount of change
    

    
    //**********perform change detection 
    if (checkThreshold(delta)){
      change = 1;
    }
    //*********

      // Serial data for each sample analysis, output for arduino data plotter
    if(serialBiodata) {
        Serial.print(averg); Serial.print(","); //dark blue
        Serial.print(stdevi); Serial.print(","); //red
        Serial.print(maxim); Serial.print(",");  //green
        Serial.print(minim); Serial.print(","); //organge
        Serial.print(delta); Serial.print(","); //purple
        Serial.print(deltaAvg + (deltaAvg*threshold)); Serial.print(","); //grey
        Serial.print((change*100) + 1000); Serial.print(","); //light blue
        Serial.print("1200"); Serial.print(","); Serial.println("0"); //black and blue
    }

    if(change) {  //generate message if change detected

      //update led light show
//      setColor(random(0,10),random(0,255),random(0,255),random(0,255), maxBright);

      //create MIDI message
      String message = "";
      MIDImessage midiMessage = { 1, nodeID,random(25,100),127,random(1060,4200),currentMillis} ;
      message = message + String(midiMessage.type) + "," + String(midiMessage.channel) +
                "," + String(midiMessage.value) + "," + String(midiMessage.velocity) +
                "," + String(midiMessage.duration) +"," + String(midiMessage.timeStamp);
      if(1) { // usbMIDI) {  //output MIDI notes through usb output
        //must keep track of any note on and time it to turn off!!!
        setNote(midiMessage);
      }

      if(speakerSound) {
 //       note.play(midiMessage.value*10, midiMessage.duration); //some note for some time
      }

      if(serverConnected) {
        if(updateServer(message)) { //send message to server
         // if(debugSerial) { Serial.print("Message Sent: "); Serial.println(message); }  
        } else {}//if(debugSerial) { Serial.print("Message FAIL: "); Serial.println(message); }
      }
      
      change = 0; //reset change
    }
  }
}




void checkInput() {

  freqPeriodCounter.poll();
  
  if (freqPeriodCounter.ready()) {
    reading = freqPeriodCounter.period; prevReading = reading;
    if(reading>0 && reading < 1200) { //limit between 0 and 1200, also manages negatives (longs dont go into int arrays)
      if(sampleIndex < sampleSize) {
        pulseInput[sampleIndex] = reading; sampleIndex++;
     }
    } 
    if(sampleIndex == sampleSize) { analyzeSample(); sampleIndex = 0; }
  } 
}



void environmentSensors() {
      // environmental data sensor readings //
    tempValue = temperature() * 1.8 + 32; // returns C modify for F //* 1.8 + 32;

//  turn off LEDs to get an ambient light reading
//  pixelsClear();
    lightValue = lightSensor();

//    Serial.print("Temp: "); Serial.print(tempValue); 
//    Serial.print(","); Serial.println(lightValue);
//    Serial.print(prevReading); Serial.print(","); Serial.println(analogRead(potPin));

}
