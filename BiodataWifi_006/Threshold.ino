
bool checkThreshold(int delta) {
//take in delta value, compare to average delta
//calculate threshold based on distance from delta-average
//return 1 if delta > threshold
  bool change = false; 
  int average = 0;

  if(deltaIndex >= deltaCount) { //argh this is crashing at deltaCount!!!

    int average = 0;
    for(int i=0;i<deltaCount;i++) { average += deltaAverage[i]; }

    average = average/deltaCount; //calculate average delta
    deltaAvg = average;
    deltaIndex = 0; 


  } 
  else { deltaAverage[deltaIndex] = delta;  deltaIndex++; } //add to array
     
  if(delta > (deltaAvg + (threshold*deltaAvg))) change = true; 
  
  return change;
}



void thresholdDown() {
  //iterate through an array of 9 threshold values
  if(threshold > thresholdMin) threshold -= thresholdAmount;
  pixelsClear();
  
  int thresh = mapfloat(threshold,thresholdMin,thresholdMax, 1, 10);
  
  for(byte i=0;i<=thresh;i++) { setColor(i,80,0,80,maxBright); }
  ledPulse(thresh,5,80,0,80,1);
  delay(550);
  pixelsClear();
}

void thresholdUp() {
  //iterate through an array of 9 threshold values
  if(threshold < thresholdMax) threshold += thresholdAmount;
  pixelsClear();
  int thresh = mapfloat(threshold,thresholdMin,thresholdMax, 1, 10);
  for(byte i=0;i<=thresh;i++) { setColor(i,0,0,255,maxBright); }
  ledPulse(thresh,5,0,0,255,1);
  delay(550);
  pixelsClear();
}
