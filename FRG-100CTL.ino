// FRG-100 Controller

#include<Wire.h>
#include<Preferences.h>
#include "EasyNextionLibrary.h"

#define TXD2 17
#define RXD2 16

Preferences frg; 
EasyNex myNex(Serial);

// Variables

// True/Fales
boolean newData = false;
// Semaphores 
//int memoryView = 0;  // Blocks GetOpData from changing n0.value when when calling memories without
                 // doing a memory recall
// Namespace variables
int checkConfig;
// Keypad (Direct Entry) global variables
String strKeypad = "";
// Up/Down step 
int step=0;
int stepsize = 10;
// Used with memories
int memCH = 2;
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
  int mode;
  if(byte1 == 0 && byte2 == 0) {  // LSB
    mode = 0;
  }
  if(byte1 == 1 && byte2 == 0) {  // USB
    mode = 1;
  }
  if(byte1 == 2 && byte2 == 0) {  // CW-Wide
    mode = 2;
  }
  if(byte1 == 0 && byte2 == 128) {  // CW-N
    mode = 3;
  }
  if(byte1 == 3 && byte2 == 0) {  // AM-W
    mode = 4;
  }
  if(byte1 == 0 && byte2 == 64) {  // AM-N
    mode = 5;
  }
  if(byte1 == 4) {  // FM
    mode = 6;
  }
  return(mode);
}

void UpdateModeDisplay(int mode, int display) {
  switch(mode) {
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
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;

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
  //GetOpData();
  //mode = EvalMode(opData[7], opData[9]);
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
  Serial2.write(CW,sizeof(CW));
  Serial2.write(band160m,sizeof(band160m));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;

}

void trigger8() {       // 80m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",3500);
  myNex.writeNum("h2.maxval",4000);
  Serial2.write(band80m,sizeof(band80m));
  Serial2.write(AM,sizeof(AM));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger9() {       // 40m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",7000);
  myNex.writeNum("h2.maxval",7300);
  Serial2.write(band40m,sizeof(band40m));
  Serial2.write(LSB,sizeof(LSB));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  //UpdateMemoryDisplay();
}

void trigger10() {      // 30m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",10100);
  myNex.writeNum("h2.maxval",10150);
  Serial2.write(band30m,sizeof(band30m));
  Serial2.write(CW,sizeof(CW));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger11() {      // 20m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",14000);
  myNex.writeNum("h2.maxval",14350);
  Serial2.write(band20m,sizeof(band20m));
  Serial2.write(USB,sizeof(USB));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger12() {      // 17m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",18068);
  myNex.writeNum("h2.maxval",18168);
  Serial2.write(band17m,sizeof(band17m));
  Serial2.write(USB,sizeof(USB));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger13() {      // 15m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",21000);
  myNex.writeNum("h2.maxval",21450);
  Serial2.write(band15m,sizeof(band15m));
  Serial2.write(USB,sizeof(USB));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger14() {      // 12m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",24890);
  myNex.writeNum("h2.maxval",24990);
  Serial2.write(band12m,sizeof(band12m));
  Serial2.write(USB,sizeof(USB));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger15() {      // 10m
  Serial.print("page page1");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff); 
  myNex.writeNum("h2.minval",28000);
  myNex.writeNum("h2.maxval",29700);
  Serial2.write(band10m,sizeof(band10m));
  Serial2.write(USB,sizeof(USB));
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger16() {              // 60m CH 1 USB
  //Serial2.write(band60mch1usb,sizeof(band60mch1usb));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(band60mch1cw,sizeof(band60mch1cw));
  //Serial2.write(CW,sizeof(CW));
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
  //Serial2.write(band60mch2usb,sizeof(band60mch2usb));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(band60mch2cw,sizeof(band60mch2cw));
  //Serial2.write(CW,sizeof(CW));
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
  //Serial2.write(band60mch3usb,sizeof(band60mch3usb));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(band60mch3cw,sizeof(band60mch3cw));
  //Serial2.write(CW,sizeof(CW));
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
  //Serial2.write(band60mch4usb,sizeof(band60mch4usb));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(band60mch4cw,sizeof(band60mch4cw));
  //Serial2.write(CW,sizeof(CW));
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
  //Serial2.write(band60mch5usb,sizeof(band60mch5usb));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(band60mch5cw,sizeof(band60mch5cw));
  //Serial2.write(CW,sizeof(CW));
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
  //Serial2.write(wwv2500,sizeof(wwv2500));
  //Serial2.write(AM,sizeof(AM));
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
  //Serial2.write(wwv5000,sizeof(wwv5000));
  //Serial2.write(AM,sizeof(AM));
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
  //Serial2.write(wwv10000,sizeof(wwv10000));
  //Serial2.write(AM,sizeof(AM));
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
  //Serial2.write(wwv15000,sizeof(wwv15000));
  //Serial2.write(AM,sizeof(AM));
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
  //Serial2.write(wwv20000,sizeof(wwv20000));
  //Serial2.write(AM,sizeof(AM));
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
  //Serial2.write(chu3330,sizeof(chu3330));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(chu7850,sizeof(chu7850));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(chu14670,sizeof(chu14670));
  //Serial2.write(USB,sizeof(USB));
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
  //Serial2.write(beacon20m,sizeof(beacon20m));
  //Serial2.write(CW,sizeof(CW));
  //GetOpData();
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
  //Serial2.write(beacon17m,sizeof(beacon17m));
  //Serial2.write(CW,sizeof(CW));
  //GetOpData();
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
  //Serial2.write(beacon15m,sizeof(beacon15m));
  //Serial2.write(CW,sizeof(CW));
  //GetOpData();
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
  //Serial2.write(beacon12m,sizeof(beacon12m));
  //Serial2.write(CW,sizeof(CW));
  //GetOpData();
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
  //Serial2.write(beacon10m,sizeof(beacon10m));
  //Serial2.write(CW,sizeof(CW));
  //GetOpData();
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
  //Serial2.write(band60mch5usb,sizeof(band60mch5usb));
  //Serial2.write(USB,sizeof(USB));
  //UpdateMemoryDisplay();
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
  //VFO2mem[3] = memCH;
  //Serial2.write(VFO2mem,sizeof(VFO2mem));
  int frequency = EvalFrequency(opData[2], opData[3], opData[4]);
  int mode = EvalMode(opData[7], opData[9]);
  String memFreq = "";
  String memMode = "";
  const char* keyFreq;
  const char* keyMode;
  memFreq = "memFreq" + memCH;
  memMode = "memMode" + memCH;
  MemoryFrequency[memCH] = frequency;
  MemoryMode[memCH] = mode;
  keyFreq = memFreq.c_str();
  keyMode = memMode.c_str();
  frg.putInt(keyFreq,frequency);
  frg.putInt(keyMode,mode);
  //int display = 0;
  //UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  //UpdateModeDisplay(MemoryMode[memCH],display);
  UpdateMemoryDisplay();
}

void trigger44() {              // M > VFO
  int display = 1;
  Frequency = MemoryFrequency[memCH];
  Mode = MemoryMode[memCH];
  SetFrequency(Frequency);
  UpdateFrequencyDisplay(Frequency, display);
  SetMode(Mode);
  UpdateModeDisplay(Mode,display);
}

void trigger45() {              // VFO mode
  Serial2.write(vfoMode,sizeof(vfoMode));
  HomePage();
  if (lastVFOfrequency != 0) {
    Frequency = savedVFOfrequency;
    Mode = savedVFOmode;
    SetFrequency(Frequency);
    SetMode(Mode);
    UpdateFrequencyDisplay(Frequency,1);
    UpdateModeDisplay(Mode,1);
    UpdateFrequencyDisplay(MemoryFrequency[memCH],0);
    UpdateModeDisplay(MemoryMode[memCH],0);
  }
}

void trigger46() {              // Memory recall
  Serial.print("page page10");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  //memCH = myNex.readNumber("n0.val");
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
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger48() {              // WWV page
  Serial.print("page page5");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger49() {              // IARU page
  Serial.print("page page4");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  GetOpData();
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger50() {              // Band slider release
  int sliderFreq = myNex.readNumber("n60.val");
  sliderFreq = sliderFreq * 100;
  SetFrequency(sliderFreq);
  UpdateFrequencyDisplay(sliderFreq,1);
}

void trigger60(){                     // 120m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",2300);
  myNex.writeNum("h2.maxval",2495);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band120m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger51(){                     // 90m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",3200);
  myNex.writeNum("h2.maxval",3400);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band90m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger52(){                     // 60m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",4750);
  myNex.writeNum("h2.maxval",4995);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band60m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger53(){                     // 49m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",5900);
  myNex.writeNum("h2.maxval",6200);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band49m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger54(){                     // 41m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",7200);
  myNex.writeNum("h2.maxval",7450);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band41m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger55(){                     // 31m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",9400);
  myNex.writeNum("h2.maxval",9900);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band31m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger56(){                     // 25m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",11600);
  myNex.writeNum("h2.maxval",12100);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band25m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger57(){                     // 22m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",13570);
  myNex.writeNum("h2.maxval",13870);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band22m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger58(){                     // 19m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",15100);
  myNex.writeNum("h2.maxval",15830);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band19m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
}

void trigger59(){                     // 16m
  Serial.print("page page3");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h2.minval",17480);
  myNex.writeNum("h2.maxval",17900);
  Serial2.write(AM,sizeof(AM));
  SetFrequency(band16m);
  GetOpData();
  int temp = int(Freq/100);
  myNex.writeNum("h2.val",temp);
  myNex.writeNum("n60.val",temp);
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;
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
  SetFrequency(target);
  HomePage();
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

}

void trigger76() {                // Sync memories from the radio
  for (int i = 1; i < 51; i++) {
    byte radio[] = {0,0,0,0,2};
    radio[3] = i;
    Serial2.write(radio,sizeof(radio));
    ReadOpData();
    int frequency = EvalFrequency(opData[2], opData[3], opData[4]);
    int mode = EvalMode(opData[7], opData[9]);
    MemoryFrequency[memCH] = frequency;                // Update the memory in RAM
    MemoryMode[memCH] = mode;
    char * memFreq = "memFreq";
    char * memMode = "memMode";
    memFreq = memFreq + i;
    memMode = memMode + i;
    frg.putInt(memFreq,frequency);            // Update the namespace values
    frg.putInt(memMode,mode);
  }
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
  //Serial2.write(AM,sizeof(AM));
  //SetFrequency(band16m);
  GetOpData();
  //myNex.writeNum("h2.val",Freq);
  //myNex.writeNum("n1.val",Freq);
  UpdateMemoryDisplay();
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
}

void trigger82() {                            // MEM CH DN
  int display = 0;
  memCH = myNex.readNumber("n0.val");
  memCH = memCH - 1;
  if (memCH < 1) {
    memCH = 50;
  }
  myNex.writeNum("n0.val",memCH);
  myNex.writeNum("h1.val",memCH);
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
  myNex.writeNum("h1.val",memCH);
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

void SetPollingIntervals() {                        // Polling interval page
  Serial.print("page page9");
  Serial.write(0xff);
  Serial.write(0xff);
  Serial.write(0xff);
  myNex.writeNum("h20.val",SmeterPollInterval);
  myNex.writeNum("n20.val",SmeterPollInterval);
  myNex.writeNum("h21.val",opDataPollInterval);
  myNex.writeNum("n21.val",opDataPollInterval);
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

void  StoreMemory() {
  int ch = myNex.readNumber("n1.val");
  int frequency = EvalFrequency(opData[2], opData[3], opData[4]);
  int mode = EvalMode(opData[7], opData[9]);
  MemoryFrequency[ch] = frequency;                // Update the memory in RAM
  MemoryMode[ch] = mode;
  String memFreq = "memFreq";
  String memMode = "memMode";
  memFreq = memFreq + ch;
  memMode = memMode + ch;
  const char* keyFreq; 
  keyFreq = memFreq.c_str();
  const char* keyMode; 
  keyMode = memMode.c_str();
  frg.putInt(keyFreq,frequency);            // Update the namespace values
  frg.putInt(keyMode,mode);
  if (updateRadioMemory == 1) {
    byte radio[] = {0,0,0,0,3};
    radio[3] = ch;
    Serial2.write(radio,sizeof(radio));
  }
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

  ReadStoredValues();                 // Read configurable values from the namespace

  LoadMemories();                     // Load stored memories


  GetOpData();
  myNex.writeNum("h1,val",memCH);
  myNex.writeNum("n0.val",memCH);
  int display = 0;
  UpdateFrequencyDisplay(MemoryFrequency[memCH],display);
  UpdateModeDisplay(MemoryMode[memCH],display);
  display = 1;

  startopDataTimer = millis();
  startTimerSmeter = millis();  

  HomePage();
}

void loop() {
  myNex.NextionListen();
  opDataTimer = millis();
  timerSmeter = millis();

  if (opDataTimer - startopDataTimer >= opDataPollInterval) {
    GetOpData();
    startopDataTimer = millis();
  }
  
  if (timerSmeter - startTimerSmeter >= SmeterPollInterval) {
    GetSmeter();
    startTimerSmeter = millis();
  }

}
