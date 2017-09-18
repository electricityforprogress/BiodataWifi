 
int wifiSetup(){
  wifi.setTransportToTCP();// this is also default
  wifi.endSendWithNewline(false); // Will end all transmissions with a newline and carrage return ie println.. default is true

  wifi_started = wifi.begin();
  if (wifi_started) { //ESP starting
    if(debugSerial) Serial.println("ESP wifi started");
    ledPulse(0,3,0,maxBright,0,1); 
    pixels.setPixelColor(0,pixels.Color(0,maxBright,0)); //ESP alive Green flash
    pixels.show();
    if(wifi.connectToAP(SSID, PASSWORD)) { //connect to wifi network
      if(debugSerial) { 
        Serial.print(SSID); Serial.print(" "); 
        Serial.print(wifi.getIP()); Serial.println(" - wifi Connected");
      }
      wifi_started++; //connected status 2
      ledPulse(2,3,0,0,maxBright,2);
      pixels.setPixelColor(2,pixels.Color(0,0,maxBright)); //Wifi network connection Blue flash
      pixels.show();

      bool serverConnect = wifi.connectToServer(SERVER_IP, SERVER_PORT); //connect to network midi server
      String message = "-->" + nodeID; // 
      message += ":0,0,0,0,0," + currentMillis; 
      message += "<--";
      serverConnect = wifi.send(SERVER, message); //send initial message
      if(serverConnect) { 
        if(debugSerial) Serial.println("server Connected"); //this isn't debugging quite rite
        pixels.setPixelColor(4,pixels.Color(maxBright,maxBright,maxBright));
        pixels.show();
        delay(1500); //hold display for a brief moment
      //turn on wifi data sending!!
        serverConnected = 1;
        
      } else { //isConnectedToServer() false ... this doesn't work quite right
          if(debugSerial) { Serial.print(wifi_started); Serial.println(" server Connection failed"); }
          ledPulse(4,5,maxBright,0,0,1);
          pixels.setPixelColor(4,pixels.Color(maxBright,0,0));
          pixels.show();
          delay(1500); //hold display for a brief moment
        //turn off wifi sending!!!
          serverConnected = 0;
        }
    } else { //wifi connect failed
      if(debugSerial) Serial.println("wifi Connection failed, sorry");
      ledPulse(2,5,maxBright,0,0,1);
      pixels.setPixelColor(2,pixels.Color(maxBright,0,0));
      pixels.show();
      delay(1500);
      }
  } else { //wifi started ESP fail
       if (debugSerial) Serial.println("ESP wifi failed, bad news!"); 
       ledPulse(0,5,maxBright,0,0,1);
       pixels.setPixelColor(0,pixels.Color(maxBright,0,0)); //red led 
       pixels.show();
       delay(1500);
    }
  
  pixelsClear(); //end wifi lightshow
  
  return wifi_started;
}

bool updateServer(String message) {
  //***************************************
  //include header and trailer to message
  // -->node:type,channel,value,velocity,duration,timeStamp<--
  // -->6:1,6,65,127,1500,453443<--
  //***************************************
  String header = "-->"; String trailer = "<--";
  String messageLong = header + String(nodeID) + ":" + message + trailer; 
  //check if connected and send message
  
    wifi.send(SERVER, messageLong); 
  int serverConnected = wifi.isConnectedToServer();  
//  if(serverConnected == 0) { wifi.send(SERVER, messageLong); }
//  else { //server not connected, flash and try to reconnect
//    pixelsClear(); 
//    ledPulse(8, 6, maxBright,0,0,5); 
//    pixelsClear();
//    wifi.connectToServer(SERVER_IP, SERVER_PORT); //connect to network midi server
//    }
//  Serial.print("serverConnected = "); Serial.println(serverConnected);

  return(serverConnected);
}

void processCommand(WifiMessage msg);

// TCP Commands
const char RST[] PROGMEM = "RST";
const char IDN[] PROGMEM = "*IDN?";

int wifiSignal() {
//gather data from CWLAP
//   +CWLAP:(3,"OxfordEast",-57,"24:a0:74:77:ff:a2",11,-31,0)
//extract SSID and RSSI
//return RSSI as a value
  String cwlap = "AT+CWLAP=\"OxfordEast\"";
  //Serial.println(cwlap);
 ESPSerial.println(cwlap);
  String inputString = "";
  String signalStrength;
  while(ESPSerial.available()){
    char inChar = (char)ESPSerial.read();
    if(inChar == '\n') { //finished reading line

      //parse inbound message and identify the SSID segment
          signalStrength = inputString.substring(24,26);
          Serial.println(inputString);
          Serial.print("RSSI:"); Serial.println(signalStrength);

     }else inputString += inChar;
  }
  return signalStrength.toInt();
}

void measureSignal(){
    int signalReading = wifiSignal();
    if(constrain(signalReading,0,90) > 0) { //ensure it is a number
     //display signal
      pixelsClear();
     for(byte i=0;i<map(constrain(signalReading,30,80),80,30,0,9);i++) { 
      if(signalReading > 75) {
       setColor(i,255,0,0,maxBright); 
      } else if (signalReading > 48) {
        setColor(i,0,255,0,maxBright);
      } else {
        setColor(i,0,0,255,maxBright);
      }
     }
     setColor(9,255,255,0,maxBright);
    }
}

