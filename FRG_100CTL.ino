// FRG-100 Controller

#include<Wire.h>
#include<Preferences.h>
#include "EasyNextionLibrary.h"

#define TXD2 17
#define RXD2 16

Preferences frg; 
EasyNex myNex(Serial);

// Variables

// Namespace variables
int checkConfig;
// Keypad (Direct Entry) global variables
String strKeypad = "";
int autoModePage = 0;
// Up/Down step 
int step=0;
int stepsize = 10;
// Used with memories
int memCH = 1;
int lastmemCH = 0;
int MemoryFrequency[51];
int MemoryMode[51];
int updateRadioMemory = 1;
int lastVFOfrequency;
int lastVFOmode;
// VFO variables
int lastFrequency;
int lastMode;
int savedVFOfrequency;
int savedVFOmode;
int Frequency;
int Mode;
// Polling
int enableFrequencyPolling = 1;
int enableSmeterPolling = 1;
// Timers
unsigned long opDataTimer;
unsigned long startopDataTimer;
unsigned long timer1;
unsigned long startTimer1;
unsigned long startTimerSmeter;
unsigned long timerSmeter=1000;
unsigned long opDataInterval=2000;
int byteCount=0;
byte vfoMode[] = {0x00,0x00,0x00,0x00,0x05};
// Software
String softwareVersion = "0.94";
String softwareBuild = "110";
// Operating Mode
byte LSB[] = {0x00,0x00,0x00,0x00,0x0C};
byte USB[] = {0x00,0x00,0x00,0x01,0x0C};
byte CW[] = {0x00,0x00,0x00,0x02,0x0C};
byte CWN[] = {0x00,0x00,0x00,0x03,0x0C};
byte AM[] = {0x00,0x00,0x00,0x04,0x0C};
byte AMN[] = {0x00,0x00,0x00,0x05,0x0C};
byte FM[] = {0x00,0x00,0x00,0x06,0x0C};
// Status updates
int opDataPollInterval = 2000;
int SmeterPollInterval = 600;
int inhibitPolling = 0;
byte readSmeter[] = {0x01,0x01,0x01,0x01,0xF7};
byte readFlags[] = {0x00,0x00,0x00,0x00,0x0FA};
byte readOpData[] = {0x00,0x00,0x00,0x02,0x10};
//byte readMemData[] = {0x04,0x04,0x04,0x04,0x10};
byte readMemData[] = {1,1,1,4,16};
//byte readAllData[] = {0x00,0x00,0x00,0x00,0x10};
//byte readAllData[]{0,0,0,0,16};
// Received data records
byte Smeter[] = {0x00,0x00,0x00,0x00,0x00};
byte opData[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};    //  19 byte Operating Data Record
byte memData[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};    //  19 byte Memory Data Record
// Memory recall
byte memRecall[] = {0,0,0,0,2};
byte m2VFO[] = {0,0,0,0,6};
byte VFO2mem[] = {0,0,0,0,3};
// Frequencies for Ham bands
int Freq;
byte band160m[] = {0X50,0x02,0x18,0x00,0x0A};     //1802.5 
byte band80m[] = {0x00,0x85,0x38,0x00,0x0A};      // 3885
byte band40m[] = {0x00,0x58,0x72,0x00,0x0A};
byte band30m[] = {0x00,0x10,0x01,0x01,0x0A};
byte band20m[] = {0x00,0x00,0x42,0x01,0x0A};
byte band17m[] = {0x00,0x25,0x81,0x01,0x0A};
byte band15m[] = {0x00,0x50,0x12,0x02,0x0A};
byte band12m[] = {0x00,0x35,0x49,0x02,0x0A};
byte band10m[] = {0x00,0x00,0x85,0x02,0x0A};
// US 60m channels
int band60mch1usb = 540350;
int band60mch1cw = 533200;
int band60mch2usb = 534650;
int band60mch2cw = 534800;
int band60mch3usb = 535700;
int band60mch3cw = 535850;
int band60mch4usb = 537150;
int band60mch4cw = 537300;
int band60mch5usb = 540350;
int band60mch5cw = 540550;

// IARU Propagation Beacons

byte beacon20m[] = {0x00,0x00,0x41,0x01,0x0A};
byte beacon17m[] = {0x00,0x10,0x81,0x01,0x0A};
byte beacon15m[] = {0x00,0x50,0x11,0x02,0x0A};
byte beacon12m[] = {0x00,0x30,0x49,0x02,0x0A};
byte beacon10m[] = {0x00,0x00,0x82,0x02,0x0A};
// SWL bands
int band120m = 230000;
int band90m = 320000;
int band60m = 475000;
int band49m = 590000;
int band41m = 720000;
int band31m = 940000;
int band25m = 1160000;
int band22m = 1357000;
int band19m = 1510000;
int band16m = 1748000;


// Functions
void SetFrequency(int newFreq) {
  String newString = "00000000";
  String tempString = "";
  byte freqArray[] = {0,0,0,0,0};             // Array to be sent to the radio
  freqArray[4] = 10;                          // Set the command byte
  if (Mode == 2 || Mode == 3) {
    newFreq = newFreq - 60;
  }
  // Convert the frequency to a string and pad with leading zeros
  String t_string = String(newFreq);
  
  int len = t_string.length();
  switch(len) {
    case 5:
      tempString = "000" + t_string;
      break;
    case 6:
      tempString = "00" + t_string;
      break;
    case 7:
      tempString = "0" + t_string;
      break;
    default:
      break;
  }
  // Change byte order and fill newString
  newString[0] = tempString[6];
  newString[1] = tempString[7];
  newString[2] = tempString[4];
  newString[3] = tempString[5];
  newString[4] = tempString[2];
  newString[5] = tempString[3];
  newString[6] = tempString[0];
  newString[7] = tempString[1];

  // Build the array
  int counter = 0;
  for (int i = 0; i < 4; i++) {
    String S1 = String(newString[counter]);
    String S2 = String(newString[counter+1]);
    int int1 = S1.toInt();
    int int2 = S2.toInt();
    int int3 = int1*16 + int2;
    freqArray[i] = byte(int3);
    counter = counter + 2;    
  }
  // Send to the radio
  Serial2.write(freqArray,sizeof(freqArray));
  Freq = newFreq;
}

int EvalFrequency(byte byte1, byte byte2, byte byte3) {
  long msb = (long)byte3 << 16;
  long middle = (long)byte2 << 8;
  long lsb = (long)byte1;
  long frequency = msb | middle | lsb;
  int Frequency = (int) frequency;
  return(Frequency);
}

int EvalMode(byte byte1, byte byte2) {
  if(byte1 == 0 && byte2 == 0) {  // LSB
    Mode = 0;
  }
  if(byte1 == 1 && byte2 == 0) {  // USB
    Mode = 1;
  }
  if(byte1 == 2 && byte2 == 0) {  // CW-Wide
    Mode = 2;
  }
  if(byte1 == 0 && byte2 == 128) {  // CW-N
    Mode = 3;
  }
  if(byte1 == 3 && byte2 == 0) {  // AM-W
    Mode = 4;
  }
  if(byte1 == 0 && byte2 == 64) {  // AM-N
    Mode = 5;
  }
  if(byte1 == 4) {  // FM
    Mode = 6;
  }
  return(Mode);
}

void UpdateModeDisplay(int Mode, int display) {
  switch(Mode) {
    case 0:
          if (display == 0) {
            myNex.writeStr("t7.txt","LSB");     // Memory display
          }
          else {
            myNex.writeStr("t2.txt", "LSB");    // VFO display
          }
          break;
    case 1:
          if (display == 0) {
            myNex.writeStr("t7.txt","USB");
          }
          else {
            myNex.writeStr("t2.txt","USB");
          }
          break;
    case 2:
          if (display == 0) {
            myNex.writeStr("t7.txt","CW");
          } else {
            myNex.writeStr("t2.txt","CW");
          }
          break;
    case 3:
          if (display == 0) {
            myNex.writeStr("t7.txt","CW-N");
          } else {
            myNex.writeStr("t2.txt","CW-N");
          }
          break;
    case 4:
          if (display == 0) {
            myNex.writeStr("t7.txt","AM");
          } else {
            myNex.writeStr("t2.txt","AM");
          }
          break;
    case 5:
          if (display == 0) {
            myNex.writeStr("t7.txt","AM-N");
          } else {
            myNex.writeStr("t2.txt","AM-N");
          }
          break;
    case 6:
          if (display == 0) {
            myNex.writeStr("t7.ttxt","FM");
          } else {
            myNex.writeStr("t2.txt","FM");
          }
          break;
    default:
          break;
  }
}

void UpdateFrequencyDisplay(int Frequency, int display) {
  if (Mode == 2 || Mode == 3){
    Frequency = Frequency + 60;
  }
    String displayFreq = "";
    String strFreq = "";
    strFreq = String(Frequency);
    int x = strFreq.length();
    switch(x) {
      case 5:
        displayFreq = String(strFreq[0]) + String(strFreq[1]) + String(strFreq[2]) + "." + String(strFreq[3]) + String(strFreq[4]);
          if (display == 0) {
            myNex.writeStr("t6.txt",displayFreq);
          } else {
            myNex.writeStr("t1.txt",displayFreq);
          }
        break;
      case 6:
        displayFreq = String(strFreq[0]) + "." + String(strFreq[1]) + String(strFreq[2]) + String(strFreq[3]) + "." + String(strFreq[4]) + String(strFreq[5]);
          if (display == 0){
            myNex.writeStr("t6.txt",displayFreq);
          } else {
            myNex.writeStr("t1.txt",displayFreq);
          }
        break;
      case 7:
        displayFreq = String(strFreq[0]) + String(strFreq[1]) + "." +String(strFreq[2]) + String(strFreq[3]) + String(strFreq[4]) + "." + String(strFreq[5]) + String(strFreq[6]);
          if (display == 0) {
            myNex.writeStr("t6.txt",displayFreq);
          } else {
            myNex.writeStr("t1.txt",displayFreq);
          }
        break;
    }
  }

void UpdateMemoryDisplay() {
  myNex.writeNum("h1.val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  //memCH = myNex.readNumber("n0.val");
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  //display = 1;

}

void ReadOpData() {
  Serial2.write(readOpData,sizeof(readOpData));
  long startTimer = millis();
  long timer = millis();
  while(timer - startTimer < 100) {    // 110ms non blocking delay to wait for data
    timer = millis();
  }
  // Populate the opData array
  int i = 0;
  while(Serial2.available() > 0) {      // popluate the array
    opData[i]=Serial2.read();
    i++;
  }
}

void ReadMemData() {
  Serial2.write(readOpData,sizeof(readOpData));
  long startTimer = millis();
  long timer = millis();
  while(timer - startTimer < 100) {    // 110ms non blocking delay to wait for data
    timer = millis();
  }
  // Populate the memData array
  int i = 0;
  while(Serial2.available() > 0) {      // popluate the array
    memData[i]=Serial2.read();
    i++;
  }
}

void GetMemory() {
  int display = 0;
  ReadMemData();
  int frequency = EvalFrequency(memData[1], memData[2], memData[3]);
  int mode = EvalMode(memData[6], memData[8]);
  UpdateFrequencyDisplay(frequency,display);
  UpdateModeDisplay(mode, display);
}

void GetOpData() {
  int display = 1;
  ReadOpData();
  Frequency = EvalFrequency(opData[2], opData[3], opData[4]);
  Mode = EvalMode(opData[7], opData[9]);
  Freq = Frequency;
  UpdateFrequencyDisplay(Frequency,display);
  UpdateModeDisplay(Mode, display);
}

void RecallMemory(int memCH) {
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  Frequency = MemoryFrequency[memCH];
  Mode = MemoryMode[memCH];
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,display);
  switch(Mode) {
    case 0:
        myNex.writeStr("t7.txt","LSB");
        break;
    case 1:
        myNex.writeStr("t7.txt","USB");
        break;
    case 2:
        myNex.writeStr("t7.txt","CW");
        break;
    case 3:
        myNex.writeStr("t7.txt","CW-N");
        break;
    case 4:
        myNex.writeStr("t7.txt","AM");
        break;
    case 5:
        myNex.writeStr("t7.txt","AM-N");
        break;
    case 6:
        myNex.writeStr("t7.txt","FM");
        break;
    default:
        break;
  }
}


void HomePage() {                       // Home Page
  Serial.print("page page0");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  GetOpData();
  UpdateMemoryDisplay();
  //myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h1.val",memCH);
  //UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  //UpdateModeDisplay(MemoryMode[memCH],0);
}

// Set Modes
void SetMode(int mode) {
  byte radio[] = {0,0,0,0,12};
  radio[3] = mode;
  Serial2.write(radio,sizeof(radio));
}

void trigger5() {    // LSB
  SetMode(0);
  UpdateModeDisplay(0,1);
}

void trigger4() {    // USB
  SetMode(1);
  UpdateModeDisplay(1,1);
}

void trigger0() {   // AM-W
  SetMode(4);
  UpdateModeDisplay(4,1);
}

void trigger1() {    // AM-N
  SetMode(5);
  UpdateModeDisplay(5,1);
}

void trigger2() {     // CW-W
  SetMode(2);
  UpdateModeDisplay(2,1);
}

void trigger3() {      // CW-N
  SetMode(3);
  UpdateModeDisplay(3,1);
}

void trigger6() {       // FM
  SetMode(6);
  UpdateModeDisplay(6,1);
}

// Ham Bands
void trigger7() {       // 160m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",1800);
  myNex.writeNum("h2.maxval",2000);
  Frequency = 180250;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;

}

void trigger8() {       // 80m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",3500);
  myNex.writeNum("h2.maxval",4000);
  Frequency = 350500;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger9() {       // 40m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",7000);
  myNex.writeNum("h2.maxval",7300);
  Frequency = 725800;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger10() {      // 30m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",10100);
  myNex.writeNum("h2.maxval",10150);
  Frequency = 1010500;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger11() {      // 20m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",14000);
  myNex.writeNum("h2.maxval",14350);
  Frequency = 1420000;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger12() {      // 17m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",18068);
  myNex.writeNum("h2.maxval",18168);
  Frequency = 1813500;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger13() {      // 15m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",21000);
  myNex.writeNum("h2.maxval",21450);
  Frequency = 2102500;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger14() {      // 12m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",24890);
  myNex.writeNum("h2.maxval",24990);
  Frequency = 2489500;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger15() {      // 10m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",28000);
  myNex.writeNum("h2.maxval",29700);
  Frequency = 2850000;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  //display = 1;
}

void trigger16() {              // 60m CH 1 USB
  Frequency = band60mch1usb;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger17() {              // 60m CH1 CW
  Frequency = band60mch1cw;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger18() {              // 60m CH 2 USB
  Frequency = band60mch2usb;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger19() {              // 60m CH 2 CW
  Frequency = band60mch2cw;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger20() {              // 60m CH 3 USB
  Frequency = band60mch3usb;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger21() {              // 60m CH 3 CW
  Frequency = band60mch3cw;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger22() {              // 60m CH 4 USB
  Frequency = band60mch4usb;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger23() {              // 60m CH 4 CW
  Frequency = band60mch4cw;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger24() {              // 60m CH 5 USB
  Frequency = band60mch5usb;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger25() {              // 60m CH 5 CW
  Frequency = band60mch5cw;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger26() {              // WWV 2.500
  Frequency = 250000;
  Mode = 4;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger27() {              // WWV 5.000
  Frequency = 500000;
  Mode = 4;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger28() {              // WWV 10.000
  Frequency = 1000000;
  Mode = 4;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger29() {              // WWV 15.000
  Frequency = 1500000;
  Mode = 4;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger30() {              // WWV 20.000
  Frequency = 2000000;
  Mode = 4;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger31() {              // CHU 3.330
  Frequency = 333000;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger32() {              // CHU 7.580
  Frequency = 758000;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger33() {              // CHU 14.670
  Frequency = 1467000;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger34() {              // IARU 20m beacon
  Frequency = 1410000;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger35() {              // IARU 17m beacon
  Frequency = 1811000;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger36() {              // IARU 15m beacon
  Frequency = 2115000;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger37() {              // IARU 12m beacon
  Frequency = 2493000;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger38() {              // IARU 10m beacon
  Frequency = 2820000;
  Mode = 2;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger39() {              // 60m band page
  Serial.print("page page2");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  Frequency = band60mch5usb;
  Mode = 1;
  SetFrequency(Frequency);
  SetMode(Mode);
  UpdateFrequencyDisplay(Frequency,1);
  UpdateModeDisplay(Mode,1);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0); 
}

void trigger40() {
  int display = 1;
  int newFreq = int(Freq - stepsize);
  SetFrequency(newFreq);
  UpdateFrequencyDisplay(newFreq, display);
}

void trigger41() {              // Step size
    if (step == 0) {
      step = 1;
      stepsize = 10;
      myNex.writeStr("b25.txt","100 Hz");
    }
    else {
      step = 0;
      stepsize = 1;
      myNex.writeStr("b25.txt","10 Hz");
    }
}

void trigger42() {
  int display = 1;
  int newFreq = int(Freq + stepsize);
  SetFrequency(newFreq);
  UpdateFrequencyDisplay(newFreq, display);
}

void trigger43() {              // VFO > M
  int Frequency = EvalFrequency(opData[2], opData[3], opData[4]);
  int Mode = EvalMode(opData[7], opData[9]);
  String memFreq = "";
  String memMode = "";
  const char* keyFreq;
  const char* keyMode;
  memFreq = "memFreq" + memCH;
  memMode = "memMode" + memCH;
  MemoryFrequency[memCH] = Frequency;
  MemoryMode[memCH] = Mode;
  keyFreq = memFreq.c_str();
  keyMode = memMode.c_str();
  frg.putInt(keyFreq,Frequency);
  frg.putInt(keyMode,Mode);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  if (updateRadioMemory == 1) {
    byte radio[] = {0,0,0,0,3};
    radio[3] = byte(memCH);
    Serial2.write(radio,sizeof(radio));
  }
}

void trigger44() {              // M > VFO
  memCH = myNex.readNumber("n0.val");
  SetFrequency(MemoryFrequency[memCH]);
  int mode = MemoryMode[memCH];
  SetMode(mode);
  GetOpData();
  UpdateFrequencyDisplay(MemoryFrequency[memCH], 0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger45() {              // VFO mode
  Serial2.write(vfoMode,sizeof(vfoMode));
  HomePage();
    Frequency = savedVFOfrequency;
    Mode = savedVFOmode;
    SetFrequency(Frequency);
    SetMode(Mode);
    UpdateFrequencyDisplay(Frequency,1);
    UpdateModeDisplay(Mode,1);
    UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
    UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger46() {              // Memory recall
  Serial.print("page page10");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  savedVFOfrequency = Frequency;
  savedVFOmode = Mode;
  myNex.writeNum("n0.val",memCH);
  myNex.writeNum("h1.val",memCH);
  int display = 0;
  RecallMemory(memCH);
  }

void trigger47() {              // SWL band page
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",15100);
  myNex.writeNum("h2.maxval",15830);
  Serial.write(AM,sizeof(AM));
  SetFrequency(band19m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger48() {              // WWV page
  Serial.print("page page5");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger49() {              // IARU page
  Serial.print("page page4");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  GetOpData();
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger50() {              // Band slider release
  int sliderFreq = myNex.readNumber("n60.val");
  sliderFreq = sliderFreq * 100;
  SetFrequency(sliderFreq);
  Mode = AutoMode(sliderFreq);
  SetMode(Mode);
  UpdateFrequencyDisplay(sliderFreq,1);
}

void trigger60(){                     // 120m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",2300);
  myNex.writeNum("h2.maxval",2495);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band120m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger51(){                     // 90m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",3200);
  myNex.writeNum("h2.maxval",3400);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band90m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger52(){                     // 60m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",4750);
  myNex.writeNum("h2.maxval",4995);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band60m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger53(){                     // 49m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",5900);
  myNex.writeNum("h2.maxval",6200);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band49m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger54(){                     // 41m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",7200);
  myNex.writeNum("h2.maxval",7450);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band41m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger55(){                     // 31m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",9400);
  myNex.writeNum("h2.maxval",9900);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band31m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger56(){                     // 25m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",11600);
  myNex.writeNum("h2.maxval",12100);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band25m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger57(){                     // 22m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",13570);
  myNex.writeNum("h2.maxval",13870);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band22m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger58(){                     // 19m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",15100);
  myNex.writeNum("h2.maxval",15830);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band19m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger59(){                     // 16m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",17480);
  myNex.writeNum("h2.maxval",17900);
  //Serial2.write(AM,sizeof(AM));
  SetFrequency(band16m);
  SetMode(4);
  GetOpData();
  int temp = int(Frequency/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1.val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
 }

void trigger61() {                    // Slider h1 release
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
}

// ---- Keypad ____________________________________________
void UpdateKeypadDisplay() {
  String strDisplayKeypad = "0000000";
  String Display = "";
  int t = strKeypad.length();
  strDisplayKeypad.remove(7-t);
  strDisplayKeypad = strDisplayKeypad + strKeypad;
  Display = String(strDisplayKeypad[0]) + String(strDisplayKeypad[1]) + "." + String(strDisplayKeypad[2]) + String(strDisplayKeypad[3]) + String(strDisplayKeypad[4]) + "." + String(strDisplayKeypad[5]) + String(strDisplayKeypad[6]);
  myNex.writeStr("t10.txt", Display);
}

void trigger62() {
  strKeypad = "";
  Serial.print("page page8");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
}

void trigger63() {                // Keypad 1 
  strKeypad = strKeypad + "1"; 
  UpdateKeypadDisplay();
}

void trigger64() {                // Keypad 2
  strKeypad = strKeypad + "2";
  UpdateKeypadDisplay();
}

void trigger65() {                // Keypad 3
  strKeypad = strKeypad + "3";
  UpdateKeypadDisplay();
}

void trigger66() {                // Keypad 4
  strKeypad = strKeypad + "4";
  UpdateKeypadDisplay();
}

void trigger67() {                // Keypad 5
  strKeypad = strKeypad + "5";
  UpdateKeypadDisplay();
}

void trigger68() {                // Keypad 6
  strKeypad = strKeypad + "6";
  UpdateKeypadDisplay();
}

void trigger69() {                // Keypad 7
  strKeypad = strKeypad + "7";
  UpdateKeypadDisplay();
}

void trigger70() {                // Keypad 8
  strKeypad = strKeypad + "8";
  UpdateKeypadDisplay();
}

void trigger71() {                // Keypad 9
  strKeypad = strKeypad + "9";
  UpdateKeypadDisplay();
}

void trigger72() {               // Keypad Enter Key
  int target = strKeypad.toInt();
  AutoMode(target);
}

void trigger73() {                // Keypad 0
  strKeypad = strKeypad + "0";
  UpdateKeypadDisplay();
}

void trigger74() {                // Keypad Erase Key
  strKeypad = "";
  UpdateKeypadDisplay();
}
// ---- End of Keypad --------------------------------------

void trigger75() {                // Sync memories to the radio
  XferControlMemories();
}

void trigger76() {                // Sync memories from the radio
  XferRadioMemories();
}

void trigger77() {                // VFO->M updates radio memory
  if (updateRadioMemory == 0) {
    updateRadioMemory = 1;
    frg.putInt("updateRadioMem",updateRadioMemory);
    myNex.writeStr("b40.txt","Enabled");
  }
  else {
    updateRadioMemory = 0;
    frg.putInt("updateRadioMem",updateRadioMemory);
    myNex.writeStr("b40.txt","Disabled");
  }
}

void trigger78() {                            // S meter polling interval
  SmeterPollInterval = myNex.readNumber("n21.val");
  frg.putInt("pollSmeter",SmeterPollInterval);
}

void trigger79() {                            // Frequency/mode polling interval
  opDataPollInterval = myNex.readNumber("n20.val");
  frg.putInt("pollRadio",opDataPollInterval);
}

void trigger80() {                            // Ham bands entry screen
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",1800);
  myNex.writeNum("h2.maxval",1999);
  GetOpData();
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
}

void trigger81() {                            // Set polling intervals page
  Serial.print("page page9");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h21.val",SmeterPollInterval);
  myNex.writeNum("n21.val",SmeterPollInterval);
  myNex.writeNum("h20.val",opDataPollInterval);
  myNex.writeNum("n20.val",opDataPollInterval);
  if (enableFrequencyPolling == 1) {
    myNex.writeStr("b23.txt","Enabled");
  }
  else {
    myNex.writeStr("b23.txt","Paused");
  }
  if (enableSmeterPolling == 1) {
    myNex.writeStr("b24.txt","Enabled");
  }
  else {
    myNex.writeStr("b24.txt","Paused");
  }
}

void trigger82() {                            // MEM CH DN
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  memCH = memCH - 1;
  if (memCH < 1) {
    memCH = 50;
  }
  myNex.writeNum("n0.val",memCH);
  myNex.writeNum("h3.val",memCH);
  RecallMemory(memCH);
}

void trigger83() {                            // MEM CH UP
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  memCH = memCH + 1;
  if (memCH > 50) {
    memCH = 1;
  }
  myNex.writeNum("n0.val",memCH);
  myNex.writeNum("h3.val",memCH);
  RecallMemory(memCH);
  }

void trigger84() {                            // H3 slider release
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  RecallMemory(memCH);
}

void trigger85() {                            // Home Button
  HomePage();
}

void trigger86() {                            // Xfer Radio Memories to FRG-100CTL
  XferRadioMemories();
}

void trigger87() {                            // Xfer FRG-100CTL Memories to Radio
  XferControlMemories();
}

void trigger88() {                            // Enable-Disable frequency polling
  if (enableFrequencyPolling == 1) {
    enableFrequencyPolling = 0;
    myNex.writeStr("b23.txt","Paused");
  }
  else {
    enableFrequencyPolling = 1;
    myNex.writeStr("b23.txt","Enabled");
  }
}

void trigger89() {                            // Enable Disable Smeter polling
  if (enableSmeterPolling == 1) {
    enableSmeterPolling = 0;
    myNex.writeStr("b24.txt","Paused");
  }
  else {
    enableSmeterPolling = 1;
    myNex.writeStr("b24.txt","Enabled");
  }
}

void trigger90() {                            // Memory Management Page
  MemoryManagement();
}

void trigger91() {                            
  AboutPage();
}

int AutoMode(int target) {
  int mode = 4;            
  int t_min = 0;
  int t_max = 0;
  autoModePage = 0;

  if (target >= 180000 && target <= 199999) {
    t_min = 1800;
    t_max = 1999;
    autoModePage = 1;
	  if( target <= 184999) {
		  mode = 2;
	  }
	  else {
		  mode = 0;
	  }
  }

  if (target >= 350000 && target <= 399999) {
    t_min = 3500;
    t_max = 3999;
    autoModePage = 1;
	  if (target <= 359999) {
		  mode = 2;
	  }
	  else {
		  mode = 0;
	  }
}

  if (target >= 700000 && target <= 729999) {
    t_min = 7000;
    t_max = 7299;
    autoModePage = 1;
    if (target <= 712499) {
      mode = 2;
    }
    else {
      mode = 0;
    }
}

  if (target >= 1010000 && target <= 1014999) {
    t_min = 10100;
    t_max = 10149;
    autoModePage = 1;
    mode = 2;
  }

  if (target >= 1400000 && target <= 1434999) {
    t_min = 14000;
    t_max = 14349;
    autoModePage = 1;
    if (target <= 1414999) {
      mode = 2;
    }
    else {
      mode = 1;
    }
  }

  if (target >= 1806800 && target <= 1816799) {
    t_min = 18068;
    t_max = 18167;
    autoModePage = 1;
    if (target <= 1810999) {
      mode = 2;
    }
    else {
      mode = 1;
    }
  }

  if (target >= 2100000 && target <= 2144999) {
    t_min = 21000;
    t_max = 21449;
    autoModePage = 1;
    if (target <= 2119999) {
      mode = 2;
    }
    else {
      mode = 1;
    }
  }

  if (target >= 2489000 && target <= 2498999) {
    t_min = 24890;
    t_max = 24989;
    autoModePage = 1;
    if (target <= 2492999) {
      mode = 2;
    }
    else {
      mode = 1;
    }
  }

  if (target >= 2800000 && target <= 2970000) {
    t_min = 28000;
    t_max = 29700;
    autoModePage = 1;
    if (target <= 2819999) {
      mode = 2;
    }
    if (target >= 2820000 && target <= 2959999) {
      mode = 1;
    }
    if (target >= 2960000) {
      mode = 6;
    }
  }

  if (target >= 230000 && target < 249500) {
    t_min = 2300;
    t_max = 2495;
    autoModePage = 2;
  }

  if (target >= 320000 && target < 340000) {
    t_min = 3200;
    t_max = 3400;
    autoModePage = 2;
  }

  if (target >= 475000 && target < 499500) {
    t_min = 4750;
    t_max = 4995;
    autoModePage = 2;
  }

  if (target >= 590000 && target < 620000) {
    t_min = 5900;
    t_max = 6200;
    autoModePage = 2;
  }

  if (target >= 730000 && target < 745000) {
    t_min = 7200;
    t_max = 7450;
    autoModePage = 2;
  }

  if (target >= 940000 && target < 990000) {
    t_min = 9400;
    t_max = 9900;
    autoModePage = 2;
  }

  if (target >= 1160000 && target < 1210000) {
    t_min = 11600;
    t_max = 12100;
    autoModePage = 2;
  }

  if (target >= 1357000 && target < 1387000) {
    t_min = 13570;
    t_max = 13870;
    autoModePage = 2;
  }

  if (target >= 1510000 && target < 1583000) {
    t_min = 15100;
    t_max = 15830;
    autoModePage = 2;
  }

  if (target >= 1748000 && target < 1790000) {
    t_min = 17480;
    t_max = 17900;
    autoModePage = 2;
  }

  switch (autoModePage) {
    case 0:
      Serial.print("page page0");
      break;
    case 1:
      Serial.print("page page1");
      break;
    case 2:
      Serial.print("page page3");
      break;
  }
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  SetMode(mode);
  SetFrequency(target);
  if (autoModePage != 0) {
    int temp = int(target/100);
    myNex.writeNum("h2.minval",t_min);
    myNex.writeNum("h2.maxval",t_max);
    myNex.writeNum("h2.val",temp);
    myNex.writeNum("n60.val",temp);
  }
  myNex.writeNum("h1.val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  UpdateFrequencyDisplay(target,1);
  UpdateModeDisplay(mode,1);
}



void GetSmeter() {                // Read the S Meter
  Serial2.write(readSmeter,sizeof(readSmeter));
  timer1 = millis();
  startTimer1 = millis();
  while (timer1 - startTimer1 <= 40) {
    timer1=millis();
  }

 int byteNum = 0;
  while(Serial2.available() > 0) {
    Smeter[byteNum]=Serial2.read();
    byteNum++;
  }
  int val = Smeter[0];
  myNex.writeNum("h0.val",val);
}

void AboutPage() {                            // About Page
  Serial.print("page page11");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeStr("t14.txt",softwareVersion);
  myNex.writeStr("t17.txt",softwareBuild);
}
void MemoryManagement() {
  Serial.print("page page7");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  if (updateRadioMemory == 1) {
    myNex.writeStr("b40.txt","Enabled");
  }
  else {
    myNex.writeStr("b40.txt","Disabled");
  }
}

void SetPollingIntervals() {                        // Polling interval page
  Serial.print("page page9");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h20.val",SmeterPollInterval);
  myNex.writeNum("n20.val",SmeterPollInterval);
  myNex.writeNum("h21.val",opDataPollInterval);
  myNex.writeNum("n21.val",opDataPollInterval);
  if (enableFrequencyPolling == 1) {
    myNex.writeStr("b23.txt","Enabled");
  }
  else {
    myNex.writeStr("b23.txt","Paused");
  }
  if (enableSmeterPolling == 1) {
    myNex.writeStr("b24.txt","Enabled");
  }
  else {
    myNex.writeStr("b24.txt","Paused");
  }
}

void UpdateMemory(long memFrequency) {
    String memFreq = "";
    String mstrFreq = "";
    mstrFreq = String(memFrequency);
    int x = mstrFreq.length();
    switch(x) {
      case 5:
        memFreq = String(mstrFreq[0]) + String(mstrFreq[1]) + String(mstrFreq[2]) + "." + String(mstrFreq[3]) + String(mstrFreq[4]);
          myNex.writeStr("t6.txt",memFreq);
        break;
      case 6:
        memFreq = String(mstrFreq[0]) + "." + String(mstrFreq[1]) + String(mstrFreq[2]) + String(mstrFreq[3]) + "." + String(mstrFreq[4]) + String(mstrFreq[5]);
          myNex.writeStr("t6.txt",memFreq);
        break;
      case 7:
        memFreq = String(mstrFreq[0]) + String(mstrFreq[1]) + "." +String(mstrFreq[2]) + String(mstrFreq[3]) + String(mstrFreq[4]) + "." + String(mstrFreq[5]) + String(mstrFreq[6]);
          myNex.writeStr("t6.txt",memFreq);
        break;
    }
  }

void InitializeNamespace() {
  frg.putInt("pollRadio",3000);
  frg.putInt("pollSmeter",1000);
  frg.putInt("updateRadioMem",0);
  frg.putInt("checkConfig",1);
  String memFreq = "memFreq";
  String memMode = "memMode";
  const char* keyFreq;
  const char* keyMode;
  for (int i = 1; i < 51; i++) {
    memFreq = "memFreq" + i;
    memMode = "memMode" + i;
    keyFreq = memFreq.c_str();
    keyMode = memMode.c_str();
    frg.putInt(keyFreq,700000);
    frg.putInt(keyMode,3);
  }
}

void ReadStoredValues() {
  opDataPollInterval = frg.getInt("pollRadio");
  SmeterPollInterval = frg.getInt("pollSmeter");
  updateRadioMemory = frg.getInt("updateRadioMem");
}

void LoadMemories() {                     // Load memory array from namespace values
  const char* keyFreq;
  const char* keyMode;
  String memMode = "";
  String memFreq = "";
  for (int i = 1; i < 51; i++) {
    memFreq = "memFreq" + i;
    memMode = "memMode" + i;
    keyFreq = memFreq.c_str();
    keyMode = memMode.c_str();
    MemoryFrequency[i] = frg.getInt(keyFreq);
    MemoryMode[i] = frg.getInt(keyMode);
    }
}

/*
void  StoreMemory() {
  MemoryFrequency[memCH] = Frequency;                // Update the memory in RAM
  MemoryMode[memCH] = Mode;
  String memFreq = "memFreq";
  String memMode = "memMode";
  memFreq = memFreq + memCH;
  memMode = memMode + memCH;
  const char* keyFreq; 
  keyFreq = memFreq.c_str();
  const char* keyMode; 
  keyMode = memMode.c_str();
  frg.putInt(keyFreq,Frequency);            // Update the namespace values
  frg.putInt(keyMode,Mode);
  if (updateRadioMemory == 1) {
    byte radio[] = {0,0,0,0,3};
    radio[3] = memCH;
    Serial2.write(radio,sizeof(radio));
  }
}
*/

void XferRadioMemories() {                // Transfer radio memories to FT-100CTL Memories
  inhibitPolling = 1;
  byte radio[] = {0,0,0,0,6};
  for (int x = 1; x < 51; x++) {
    radio[3] = byte(x);
    memCH = x;
    Serial2.write(radio,sizeof(radio));
    ReadOpData();
    Frequency = EvalFrequency(opData[2], opData[3], opData[4]);
    Mode = EvalMode(opData[7], opData[9]);
    MemoryFrequency[memCH] = Frequency;
    MemoryMode[memCH] = Mode;
    String memFreq = "memFreq";
    String memMode = "memMode";
    memFreq = memFreq + memCH;
    memMode = memMode + memCH;
    const char* keyFreq; 
    keyFreq = memFreq.c_str();
    const char* keyMode; 
    keyMode = memMode.c_str();
    frg.putInt(keyFreq,Frequency);            // Update the namespace values
    frg.putInt(keyMode,Mode);
  }
  inhibitPolling = 0;
}

void XferControlMemories() {                // Transfer Frg-100CTL memories to the radio
  inhibitPolling = 1;
  byte radio[] = {0,0,0,0,3};
  for (int x = 1; x < 51; x++) {
    memCH = x;
    radio[3] = byte(x);
    SetFrequency(MemoryFrequency[memCH]);
    SetMode(MemoryMode[memCH]);
    Serial2.write(radio,sizeof(radio));
  }
  inhibitPolling = 0;
}


void setup() {
  frg.begin("FRG-100",false);
  Serial.begin(9600);
  myNex.begin(9600);
  Serial2.begin(4800, SERIAL_8N2, RXD2, TXD2);

  checkConfig = frg.getInt("checkConfig",0);
  if (checkConfig != 1) {
    InitializeNamespace();
  }

  ReadStoredValues();                     // Read configurable values from the namespace
  LoadMemories();                         // Load stored memories
  Serial2.write(vfoMode,sizeof(vfoMode)); // Force VFO Mode
  HomePage();
  GetOpData();
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
  UpdateModeDisplay(MemoryMode[memCH],0);
  startopDataTimer = millis();
  startTimerSmeter = millis();  
}

void loop() {
  myNex.NextionListen();
  opDataTimer = millis();
  timerSmeter = millis();
  
  if (inhibitPolling == 0) {
    if (opDataTimer - startopDataTimer >= opDataPollInterval) {
      if (enableFrequencyPolling == 1) {
        inhibitPolling = 1;
        GetOpData();
        inhibitPolling = 0;
      }
      startopDataTimer = millis();
    }
  }
  
  if (inhibitPolling == 0) {
      if (timerSmeter - startTimerSmeter >= SmeterPollInterval) {
        if (enableSmeterPolling == 1) {
          inhibitPolling = 1;
          GetSmeter();  
          inhibitPolling = 0;
        }
        startTimerSmeter = millis();
      }
  }

}
