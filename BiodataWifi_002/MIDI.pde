
void sendMIDI(int type, int channel, int value, int velocity) {
 if(midiOn==1) {
    if(type==1) {
      if(velocity > 0) midi.sendNoteOn(channel,value,velocity); //note on
      else midi.sendNoteOff(channel,value,velocity); //note off
    } else if(type==2) midi.sendControllerChange(channel,value,velocity); //cc controller
 }
}

void checkMIDI() {
  if(midiOn==1){ //if toggled on, saves time and bandwidth
  //iterate through all objects and send updated MIDI messages 
     for(byte i=0;i<nodeCount;i++) {
      for(byte j=0;j<polyphony;j++) {
          if(nodes[i].velocity[j] > 0 && nodes[i].isPlaying[j] == 0) { //pending notes
            if(nodes[i].eventTime[j] <= currentMillis) { //event time trigger for pending note
              sendMIDI(1,nodes[i].channel[j]-1,nodes[i].note[j],nodes[i].velocity[j]); //send timed note
              nodes[i].isPlaying[j] = 1;
              //print("Note On NodeID:"); print(i+1); print(" "); print(nodes[i].note[j]); 
              //print(" poly: "); print(j); //note number
              //print(" currentMillis: "); print(currentMillis);  print(" "); print(nodes[i].eventTime[j]);
              //println();
            } 
          }
          else if(nodes[i].velocity[j] > 0 && nodes[i].isPlaying[j] == 1) { //currently playing
                if(nodes[i].duration[j] < currentMillis) { //note is expired
                  nodes[i].velocity[j] = 0; //reset velocity
                  nodes[i].isPlaying[j] = 0 ; //reset scheduler
                  //leave other data for potential future analysis 
                  sendMIDI(1,nodes[i].channel[j]-1,nodes[i].note[j],0); //note off
                  //print("Note Off NodeID:"); print(i+1); print(" "); print(nodes[i].note[j]); println();
                }
          } 
       }//for polyphony
    }//for nodeCount
    
    //print all node data summary
    if(explicitDebug){
      //only print every few milliseconds
      if(currentMillis - prevMillis > 250) { //only print out periodically, slow it all down
        prevMillis = currentMillis; //update time
        //header -- each 
          int mil = int(currentMillis);
          int s = second();  // Values from 0 - 59
          int m = minute();  // Values from 0 - 59
          int h = hour();    // Values from 0 - 23
          
        print("{"); print(h); print(":"); print(m); print(":"); print(s); print("} "); print(Server.ip());
        print(" "); print(messageError); print(" "); print(insplitError); print(" "); println();
        for(byte i=0;i<nodeCount;i++){
        print("["); print(i+1); print("]("); //node header
        if(i<nodeCount-1) print(" "); //single white space to align with [1]-[10]
          for(byte j=0;j<polyphony;j++) {
            if(nodes[i].velocity[j] > 0) {
              if(nodes[i].isPlaying[j] == 0) print("*"); //pending
              else print (1); //playing
            } else print ("-"); //empty
          }
          println(")"+" IP-"+nodes[i].ipAddress);
        }
        println(); //give an extra line return
      }
    }// explicitDebug
    
    
  }//midiOn == 1
}//checkMIDI()

boolean addNote(int nodeID, int channel, int value, int velocity, long duration, long timeStamp) {
 
  //find any 'silent' notes in the array (vel = 0), insert for management
  for(byte i=0;i<polyphony;i++) {
    if(nodes[nodeID-1].velocity[i] == 0) {
       nodes[nodeID-1].channel[i] = channel;
       nodes[nodeID-1].note[i] = value;
       nodes[nodeID-1].velocity[i] = velocity;
       nodes[nodeID-1].duration[i] =  BUFFER + nodes[nodeID-1].timingOffset + +timeStamp + duration; //end time
       nodes[nodeID-1].eventTime[i] = BUFFER + nodes[nodeID-1].timingOffset + timeStamp; //start time
       nodes[nodeID-1].isPlaying[i] = 0; 
       return true; //note placed
    }
  }
  return false; // no empty notes
}