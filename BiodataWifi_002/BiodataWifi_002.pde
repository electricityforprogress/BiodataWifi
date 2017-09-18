// Creates a server that prints new client's IP addresses. 
import themidibus.*; 
import processing.net.*;
import java.util.*;

MidiBus midi; 

int midiOn = 1; //toggle MIDI output 
boolean explicitDebug = true; //toggle explicit debugging

int nodeCount = 10;

int BUFFER = 5000;// 20000; //an attempt at buffering and scheduling the inbound MIDI notes so nodes are self synched

int rateFast = 1; //fast rate, smoothing transition values for 'control' message
int rateSlow = 3; //default rate
int midiAmt = 7; // additive cc modifier, makes the cc fast and jumpy or slow and smooth

String nodeError = "Node: --- ";
String messageError = "Message: --- ";
String insplitError = "inSplit: --- ";
long errorMillis = 0;

int port = 1234;   
Server myServer;     
Client c;

long currentMillis = 0;
long prevMillis = 0;

int polyphony = 100; //maximum number of notes to generate and track from inbound node data

public class node {
 public int nodeID;
 public long timingOffset; //synchronize inbound messages, every day i'm buffering
 public int ipAddress;
 public int[] note = new int[polyphony];  
 public int[] channel = new int[polyphony];
 public int[] velocity = new int[polyphony];
 public long[] duration = new long[polyphony]; //note length or control rate
 public long[] eventTime = new long[polyphony]; //used to sync if buffering multiple nodes  
 public int[] isPlaying = new int[polyphony]; //track notes for buffering
 public float temperature; // Temperature
 public int lightSensor; // 0 - 1023 ~1500 Lux
 
 
  node() {
   nodeID = 0;
   temperature = 0;
   lightSensor = 0;
   timingOffset = 0;
   ipAddress = 0;
   for(byte i=0;i<note.length;i++) note[i] = 0;
   for(byte i=0;i<channel.length;i++) channel[i] = 0;
   for(byte i=0;i<velocity.length;i++) velocity[i] = 0;
   for(byte i=0;i<duration.length;i++) duration[i] = 0;
   for(byte i=0;i<eventTime.length;i++) eventTime[i] = 0;
   for(byte i=0;i<isPlaying.length;i++) isPlaying[i] = 0;
  }
  
}

node nodes[] = new node[nodeCount]; //array of node objects which we will be tracking locally

String input;


void setup() {
  size(120,100,P2D);
  frameRate(20);

  for(int i=0;i<nodeCount;i++) { nodes[i] = new node(); } //initialize those nodes!

  MidiBus.list();
  midi = new MidiBus(this, -1, "IAC Bus 1");
  //midi = new MidiBus(this, -1, "Test");
  
  for(byte i=0;i<nodeCount;i++) nodes[i].nodeID = i+1; //setup node array

  myServer = new Server(this, port); // Starts a server on port 1234
  print("Server is live at ... "); println(Server.ip());
  
}

void draw() {
  
  currentMillis = millis(); //manage global timeclock, easily identify actions in current cycle
  
  readInputs();  //read from clients and parse out their message, updating objects
  
  //implement a rolling buffer in order to 're synch' the data streams of each node, timing is everything!
  
  checkMIDI(); //read current time and objects and update MIDI output 
  
  //clearErrors(); //clear error log with time

}