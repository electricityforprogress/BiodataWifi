

// ServerEvent message is generated when a new client connects 
// to an existing server.
void serverEvent(Server someServer, Client someClient) {
  int mil = int(currentMillis);
  int s = second();  // Values from 0 - 59
  int m = minute();  // Values from 0 - 59
  int h = hour();    // Values from 0 - 23
 
  //print(h); print(":"); print(m); print(":"); print(s); 
  //println(" Node Connected at IP: " + someClient.ip());

  //search if IP has been registered previously, reset timingOffset
  String[] ipSplit = split(someClient.ip(),'.'); //extract IP address from Client for identification
  String clientID = "";
  if(ipSplit.length == 4) clientID = ipSplit[3]; 
  for(byte i=0;i<nodeCount;i++) { 
    if(nodes[i].ipAddress == PApplet.parseInt(clientID)) {
      nodes[i].timingOffset = 0;  //reset offset if reconnecing 
      //print("Reconnecting IP-"); print(clientID); print(" NodeID: "); println(nodes[i].nodeID);
      messageError = "Client ReConnect: " + clientID + " NodeID: " + nodes[i].nodeID+  " {"+ h + ":" + m + ":" + s + "} ";
    }
    if(i==nodeCount-1) { //no match in list
      messageError = "Client: " + clientID + " {" + h + ":" + m + ":" + s + "} ";   
    }
  }
  
}

void disconnectEvent(Client c) {
  String dataIn = c.ip();
  String[] inSplit = split(c.ip(),'.');
  messageError = "Client Disconnect: " + inSplit[3]; //print client IP
  //print("Client Disconnect:  ");
  //println(dataIn);
  //background(dataIn);
}


void readInputs() {
//update object data based on client input identification 
 int mil = int(currentMillis);
 int s = second();  // Values from 0 - 59
 int m = minute();  // Values from 0 - 59
 int h = hour();    // Values from 0 - 23
  
// Receive data from client
 c = myServer.available();
 if (c != null) { //client has posted data to be read
   input = c.readString();
  if(input.length() > 15) { //minimum message input length
   //identify header and trailer to ensure clean message!!!
   String header = "-->"; String trailer = "<--";   
   String inputHeader = input.substring(0,3); //first three chars
   String inputTrailer = input.substring(input.length()-3,input.length()); //last three chars 
   String clientID = "0";
   
  if(header.equals(inputHeader) && trailer.equals(inputTrailer)) { //valid message

   //check for multiple messages and trim 
   String singleMessage[] = split(input, "<");
   if(singleMessage[1].length() != 3) input = singleMessage[0] + "<--";   //more than one message String in the input
   //trim off header and trailer
   input = input.substring(3,input.length()-3);
   
   String[] ipSplit = split(c.ip(),'.'); //extract IP address from Client for identification
   if(ipSplit.length == 4) clientID = ipSplit[3];
   else return; //bad IP
   
   //print("IP-"); print(clientID); print(" "); 
   //print(h); print(":"); print(m); print(":"); print(s); //print(":"); print(mil); 
   //print(" "); //println(input);
    
   
   //parse input into NodeID and sensor values
   String[] inSplit = split(input,':');
   //check inSplit array length, incase of bad data values, don't abend
   if(inSplit.length == 2) {  //error debug at inSplit
     String nodeIDstr = inSplit[0];
     String[] messageData = split(inSplit[1],',');
       int nodeID = 0;
       int type = 0;
       int channel = 0;
       int value = 0;
       int velocity = 0;
       long duration = 0; //these need to be longs
       long timeStamp = 0; //these need to be longs
     if(messageData.length == 6) { //error debug for bad message inbound
       for(byte i=0;i<messageData.length;i++) { if(messageData[i]=="") messageData[i]="0"; } //clean data
    //numberformatexception: for input string: ""
    //argh!
       nodeID = PApplet.parseInt(nodeIDstr);
       type = PApplet.parseInt(messageData[0]);
       channel = PApplet.parseInt(messageData[1]);
       value = PApplet.parseInt(messageData[2]);
       velocity = PApplet.parseInt(messageData[3]);
       duration = Long.parseLong(messageData[4]); //these need to be longs
       timeStamp = Long.parseLong(messageData[5]); //these need to be longs

       //synchronize the timing offset on first note in each node - a rough synch at best, but can buffer
       if(nodeID > 0 && nodeID < nodeCount) { //ensure valid node - minor data sanity
         if(nodes[nodeID-1].ipAddress == 0) { //no ip address first time
           nodes[nodeID-1].ipAddress = PApplet.parseInt(clientID); //set IP of node, manage disconnect?
           //reset previous node 
         }
         if(nodes[nodeID-1].timingOffset == 0) {
           if(currentMillis >= timeStamp) { //if server time above node
             nodes[nodeID-1].timingOffset = currentMillis - timeStamp;
           } else nodes[nodeID-1].timingOffset = currentMillis - timeStamp; //node above server
           
           //if re-connecting a node, need to rebuild offset ...
          // println(); print("Timing Offset:"); print(nodes[nodeID-1].timingOffset); print(" Node:"); println(nodeID);
         }
         
             //Add note, if open polyphony is available
         if(addNote(nodeID,channel,value,velocity,duration,timeStamp)) {
           //print(" "); print("      Note Added");
          } else {}//print("     Note NOT Added");
               
          
       } else { } // nodeError = clientID + "-Node: " + nodeID + " {" + currentMillis + "}"; errorMillis = currentMillis; } // println("NodeID Error!!");
     } else { messageError = clientID + "-Message: " + inSplit[1] + " {" + h + ":" + m + ":" + s + "}"; errorMillis = currentMillis; }//print("messageData Parsing Error:  " + input);
     //println();
    } else { insplitError = clientID + "-inSplit: " + input + " {" + h + ":" + m + ":" + s + "}"; errorMillis = currentMillis; } // { print("Message inSplit Parsing Error: "); println(input); }//keep getting out of bounds
  } else { nodeError = clientID + "-HeadTail: " + input + " {" + h + ":" + m + ":" + s + "} " + inputHeader + " " + inputTrailer; } // header trailer check
 }//input.length() > 15 //minimum message size
 }// client data available
}

void clearErrors() {
  if(errorMillis + 10000 > currentMillis) { //clear errors after 10 seconds
   nodeError = "Node: --- ";
   messageError = "Message: --- ";
   insplitError = "inSplit: ---  ";
  }
}