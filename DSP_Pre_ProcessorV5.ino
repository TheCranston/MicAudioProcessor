/* TODO
   Tone generator
*/

#include <EEPROM.h> // store the config here for power persistance
#include <SD.h>
#include <SPI.h>
#include <Audio.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>  // goal is to migrate to pjrc's optimized library


#include <TimerOne.h>
#include <ClickEncoder.h>
#include <menu.h>
#include <menuIO/serialIO.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/adafruitGfxOut.h>

#include "effect_dynamics.h"

File myFile;
int myPreset = 0;
const int chipSelect = 10;
char charRead;

const float DEG2RAD = PI / 180.0f;

using namespace Menu;

//Define your font here. Default font: lcd5x7
//#define menuFont X11fixed7x14
#define fontW 7
#define fontH 14

// Encoder /////////////////////////////////////
#define encA 4
#define encB 3
#define encBtn 5
ClickEncoder clickEncoder(encA, encB, encBtn, 2);
ClickEncoderStream encStream(clickEncoder, 1);
MENU_INPUTS(in, &encStream);
void timerIsr() {
  clickEncoder.service();
}

AudioControlSGTL5000     audioShield;
AudioInputI2S            audioInput;
AudioOutputI2S           audioOutput;
AudioAnalyzePeak         peakPre;
AudioAnalyzePeak         peakPost;
AudioAnalyzeFFT256       fftValues;
AudioEffectDynamics      Dynamics;
AudioFilterBiquad        EQ_1;
AudioFilterBiquad        EQ_2;
AudioFilterBiquad        EQ_3;
AudioFilterBiquad        EQ_4;
AudioFilterBiquad        EQ_5;
AudioFilterBiquad        EQ_6;
AudioFilterBiquad        EQ_7;
AudioFilterBiquad        EQ_8;
AudioMixer4              EQ_Mix1;
AudioMixer4              EQ_Mix2;
AudioMixer4              EQ_MixOut;
AudioConnection          patchCord1(audioInput, 0, peakPre, 0);
AudioConnection          patchCord3(audioInput, 0, EQ_MixOut, 2);
AudioConnection          patchCord4(audioInput, 0, EQ_1, 0);
AudioConnection          patchCord5(audioInput, 0, EQ_2, 0);
AudioConnection          patchCord6(audioInput, 0, EQ_3, 0);
AudioConnection          patchCord7(audioInput, 0, EQ_4, 0);
AudioConnection          patchCord8(audioInput, 0, EQ_5, 0);
AudioConnection          patchCord9(audioInput, 0, EQ_6, 0);
AudioConnection          patchCord10(audioInput, 0, EQ_7, 0);
AudioConnection          patchCord11(audioInput, 0, EQ_8, 0);
AudioConnection          patchCord15(EQ_1, 0, EQ_Mix1, 0);
AudioConnection          patchCord16(EQ_2, 0, EQ_Mix1, 1);
AudioConnection          patchCord14(EQ_3, 0, EQ_Mix1, 2);
AudioConnection          patchCord13(EQ_4, 0, EQ_Mix1, 3);
AudioConnection          patchCord12(EQ_5, 0, EQ_Mix2, 0);
AudioConnection          patchCord17(EQ_6, 0, EQ_Mix2, 1);
AudioConnection          patchCord18(EQ_7, 0, EQ_Mix2, 2);
AudioConnection          patchCord19(EQ_8, 0, EQ_Mix2, 3);
AudioConnection          patchCord20(EQ_Mix1, 0, EQ_MixOut, 0);
AudioConnection          patchCord21(EQ_Mix2, 0, EQ_MixOut, 1);
AudioConnection          patchCord22(EQ_MixOut, 0, Dynamics, 0);
AudioConnection          patchCord23(Dynamics, fftValues);
AudioConnection          patchCord24(Dynamics, peakPost);
AudioConnection          patchCord25(Dynamics, 0, audioOutput, 0);
AudioConnection          patchCord26(Dynamics, 0, audioOutput, 1);

//Auto Volume Control (AVC) on
/* Valid values for dap_avc parameters
  maxGain; Maximum gain that can be applied
  0 - 0 dB
  1 - 6 dB
  2 - 12 dB
  lbiResponse; Integrator Response
  0 - 0 mS
  1 - 25 mS
  2 - 50 mS
  3 - 100 mS
  hardLimit
  0 - Hard limit disabled. AVC Compressor/Expander enabled.
  1 - Hard limit enabled. The signal is limited to the programmed threshold (signal saturates at the threshold)
  threshold
  floating point in range 0 to -96 dB
  attack
  floating point figure is dB/s rate at which gain is increased
  decay
  floating point figure is dB/s rate at which gain is reduced
*/

//DEFAULT FLAGS
#define AVCFLAG 0;
#define EQUALIZERFLAG 0;
#define NOISEGATEFLAG 0;
#define MYINPUT AUDIO_INPUT_MIC;

//DEFAULT AVC LEVELS
#define AVCGAIN 1.0f;
#define MYAVCGAIN 1;
#define MYAVCRESP 1;
#define MYAVCHARD 0;
#define MYAVCTHR -18.0f;
#define MYAVCATT 0.5f;
#define MYAVCDEC 1.0f;

//DEFAULT LEVELS
#define MYLINEINLEVEL 0;
#define MYLINEOUTLEVEL 29;
#define MYVOLUME 0.7f;
#define MICGAINSET 35;

// DEFAULT NOISE GATE PARAMETERS
#define MYNGATTACKTIME 0.01f;
#define MYNGRELEASETIME 0.2f;
#define MYNGTHRESHOLD -110.0f;
#define MYNGHYSTERISIS 6.0f;
#define MYNGHOLDTIME 0.0003f;

// DEFAULT PROCESSOR PARAMETERS
#define PROCFLAG 0;
#define MYPRCTHRESHOLD -30.0f;
#define MYPRCATTACK 0.03f;
#define MYPRCRELEASE 0.5f;
#define MYPRCRATIO 8.0f;
#define MYPRCKNEEWIDTH 6.0f;

// DEFAULT LIMITER PARAMETERS
#define LIMFLAG 0;
#define MYLIMTHRESHOLD -3.0f;
#define MYLIMATTACK 0.03f;
#define MYLIMRELEASE 0.03f;

// DEFAULT AUTO MAKEUP GAIN PARAMETERS
#define AMGFLAG 0;
#define MYAMGHEADROOM 6.0f;

// DEFAULT MAKEUP GAIN PARAMETERS
#define MUPFLAG 0;
#define MYMUPGAIN 0.0f;

// EEPROM Magic - if present then we have a config to load
int EEPROM_MAGIC[4] = {0xFE,0xED,0xC0,0xDE};  // https://en.wikipedia.org/wiki/Hexspeak for the LOLs
int EEPROM_INIT[4] = {0x00,0x00,0x00,0x00}; // invalid config
int tempMagic[4] = {0x00,0x00,0x00,0x00};

struct ConfigSaveSet {
   float AVCgain;
   int AVCFlag;
   int equalizerFlag;
   int myLineInLevel;
   int myLineOutLevel;
   float myVolume;
   float micGainSet;
   int myAVCGain;
   int myAVCResp;
   int myAVCHard;
   float myAVCThr;
   float myAVCAtt;
   float myAVCDec;
   int myInput;
   float ydBLevel[8];
   float myNGattackTime;
   float myNGreleaseTime;
   float myNGthreshold;
   float myNGholdTime;
   int noiseGateFlag;
   float myNGhysterisis;
   int procFlag;
   float myPRCthreshold;
   float myPRCattack;
   float myPRCrelease;
   float myPRCratio;
   float myPRCkneeWidth;
   int limFlag;
   float myLIMthreshold;
   float myLIMattack;
   float myLIMrelease;
   int amgFlag;
   float myAMGheadroom;
   int mupFlag;
   float myMUPgain;
} mySaveSet;

int b;
int maxVal = 0;
int spectrumFlag = 0;

float AVCgain = AVCGAIN;
int AVCFlag = AVCFLAG;

int myAVCGain = MYAVCGAIN;
int myAVCResp = MYAVCRESP;
int myAVCHard = MYAVCHARD;
float myAVCThr = MYAVCTHR;
float myAVCAtt = MYAVCATT;
float myAVCDec = MYAVCDEC;

int freqBand;
const int eqFreq[] = {150, 240, 370, 590, 900, 1300, 2000, 3300};

float bandGain[] = {1, 1, 1, 1, 1, 1, 1, 1};
float ydBLevel[] = {0, 0, 0, 0, 0, 0, 0, 0};

int equalizerFlag = EQUALIZERFLAG;
int noiseGateFlag = NOISEGATEFLAG;
int myLineInLevel = MYLINEINLEVEL;    // range is 0 to 15
int myLineOutLevel = MYLINEOUTLEVEL;  // range is 13 to 31
float myVolume = MYVOLUME;     // 0.8 is max for undistorted headphone output
float micGainSet = MICGAINSET;    // 35 for the Chinese MH-1B8 mic
int myInput = MYINPUT;

// Default Noise Gate parameters
float myNGattackTime = MYNGATTACKTIME;
float myNGreleaseTime = MYNGRELEASETIME;
float myNGthreshold = MYNGTHRESHOLD;
float myNGhysterisis = MYNGHYSTERISIS;
float myNGholdTime = MYNGHOLDTIME;

// Default Processor parameters
int procFlag = PROCFLAG;
float myPRCthreshold = MYPRCTHRESHOLD;
float myPRCattack = MYPRCATTACK;
float myPRCrelease = MYPRCRELEASE;
float myPRCratio = MYPRCRATIO;
float myPRCkneeWidth = MYPRCKNEEWIDTH;

// Default Limiter parameters
int limFlag = LIMFLAG;
float myLIMthreshold = MYLIMTHRESHOLD;
float myLIMattack = MYLIMATTACK;
float myLIMrelease = MYLIMRELEASE;

// Default Auto Makeup Gain parameters
int amgFlag = AMGFLAG;
float myAMGheadroom = MYAMGHEADROOM;

// Default Makeup Gain parameters
int mupFlag = MUPFLAG;
float myMUPgain = MYMUPGAIN;

float barPeak[20];
float xPreNeedleOld;
float yPreNeedleOld;
float xPostNeedleOld;
float yPostNeedleOld;
float peakPreROld;
float peakPostROld;

const uint8_t fftOctTab[] = {
  1,  1,
  2,  2,
  3,  3,
  4,  4,
  5,  5,
  6,  7,
  8,  9,
  10, 11,
  12, 14,
  14, 16,
  17, 20,
  21, 25,
  26, 31,
  32, 38,
  39, 46,
  47, 57,
  58, 69,
  70, 85,
  86, 127
};

// CSL
#define UC_Width 320
#define UC_Height 240

#ifdef ESP8266
  #define UC_CS   14
  #define UC_DC   9
  #define UC_RST  22
#else
  #define UC_CS   14
  #define UC_DC   9
  #define UC_RST  22
#endif

//define colors
const colorDef<uint16_t> my_colors[6] MEMMODE={
  {{(uint16_t)ILI9341_BLACK,    (uint16_t)ILI9341_BLACK},     {(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLUE,  (uint16_t)ILI9341_BLUE}},//bgColor
  {{(uint16_t)ILI9341_DARKGREY, (uint16_t)ILI9341_DARKGREY},  {(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_WHITE}},//fgColor
  {{(uint16_t)ILI9341_WHITE,    (uint16_t)ILI9341_BLACK},     {(uint16_t)ILI9341_YELLOW,(uint16_t)ILI9341_YELLOW,(uint16_t)ILI9341_RED}},//valColor
  {{(uint16_t)ILI9341_WHITE,    (uint16_t)ILI9341_BLACK},     {(uint16_t)ILI9341_WHITE, (uint16_t)ILI9341_YELLOW,(uint16_t)ILI9341_YELLOW}},//unitColor
  {{(uint16_t)ILI9341_WHITE,    (uint16_t)ILI9341_DARKGREY},  {(uint16_t)ILI9341_BLACK, (uint16_t)ILI9341_BLUE,  (uint16_t)ILI9341_WHITE}},//cursorColor
  {{(uint16_t)ILI9341_WHITE,    (uint16_t)ILI9341_YELLOW},    {(uint16_t)ILI9341_BLUE,  (uint16_t)ILI9341_RED,   (uint16_t)ILI9341_RED}},//titleColor
};

// Setting the screen driver for the Spectrum Display
Adafruit_ILI9341 display = Adafruit_ILI9341(UC_CS, UC_DC, UC_RST);  // Using standard MOSI=11, SCLK=13, MISO=12  - Specify if using alternate pins

// result doAlert(eventMask e, prompt &item);
eventMask evt;

result alert(menuOut& o, idleEvent e) {
  if (e == idling) {
    //Serial.println("idling");
    display.setCursor(0, 0);
    display.print("alert test");
    display.setCursor(0, 10);
    display.print("press [select]");
    display.setCursor(0, 20);
    display.print("to continue...");
  }
  return proceed;
}

//Equalizer on
result eqON(eventMask e) {
  EQ_MixOut.gain(0, 1);
  EQ_MixOut.gain(1, 1);
  EQ_MixOut.gain(2, 0);
  EQ_MixOut.gain(3, 0);
  //Serial.println(""); //Serial.print(e); //Serial.println(" eqON executed, proceed menu"); //Serial.flush();
  return proceed;
}

//Equalizer off
result eqOFF(eventMask e) {
  EQ_MixOut.gain(0, 0);
  EQ_MixOut.gain(1, 0);
  EQ_MixOut.gain(2, 1);
  EQ_MixOut.gain(3, 0);
  //Serial.println(""); //Serial.print(e); //Serial.println(" eqOFF executed, proceed menu"); //Serial.flush();
  return proceed;
}

result AVCon(eventMask e) {
  AVCFlag = 1;
  audioShield.autoVolumeControl( myAVCGain    // Maximum gain that can be applied 0 - 0 dB / 1 - 6.0 dB / 2 - 12 dB
                                 , myAVCResp  // Integrator Response 0 - 0 mS / 1 - 25 mS / 2 - 50 mS / 3 - 100 mS
                                 , myAVCHard  // hardLimit
                                 , myAVCThr   // threshold floating point in range 0 to -96 dB
                                 , myAVCAtt   // attack floating point figure is dB/s rate at which gain is increased
                                 , myAVCDec); // decay floating point figure is dB/s rate at which gain is reduced
  audioShield.autoVolumeEnable();
  //Serial.println(""); //Serial.print(e); //Serial.println(" AVCon executed, proceed menu"); //Serial.flush();
  return proceed;
}

//AVC off
result AVCoff(eventMask e) {
  AVCFlag = 0;
  audioShield.autoVolumeDisable();
  //Serial.println(""); //Serial.print(e); //Serial.println(" AVCoff executed, proceed menu"); //Serial.flush();
  return proceed;
}

MENU(subLevels, "Vol/Lim Levels", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD(     micGainSet,     "Mic.Gain", " ",   0, 63,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD(       myVolume,    "Headphone", " ",   0,  1, 0.1, 0.01, SetLevels, updateEvent, noStyle)
     , FIELD(  myLineInLevel,      "Line In", " ",   0, 15,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD( myLineOutLevel,     "Line Out", " ",  13, 31,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD(  myAMGheadroom, "AMG Headroom", " ",   0, 60,   1,     , SetLevels, updateEvent, noStyle)
     , FIELD(      myMUPgain,  "Makeup Gain", " ", -12, 24,   1,     , SetLevels, updateEvent, noStyle)
    );

TOGGLE(myAVCGain, chooseAVCgain, "AVC Gain: ", doNothing, noEvent, wrapStyle
       , VALUE( "0 dB", 0, SetAVCParameters, updateEvent)
       , VALUE( "6 dB", 1, SetAVCParameters, updateEvent)
       , VALUE("12 dB", 2, SetAVCParameters, updateEvent)
      );

TOGGLE(myAVCResp, chooseAVCresp, "Response: ", doNothing, noEvent, wrapStyle
       , VALUE(  "0 ms", 0, SetAVCParameters, updateEvent)
       , VALUE( "25 ms", 1, SetAVCParameters, updateEvent)
       , VALUE( "50 ms", 2, SetAVCParameters, updateEvent)
       , VALUE("100 ms", 3, SetAVCParameters, updateEvent)
      );

TOGGLE(myAVCHard, setHardLimit, "Hard Limit: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, SetAVCParameters, updateEvent)
       , VALUE("Off", 0, SetAVCParameters, updateEvent)
      );

MENU(subAVC, "Auto Vol Ctl cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , SUBMENU(chooseAVCgain)
     , SUBMENU(chooseAVCresp)
     , SUBMENU(setHardLimit)
     , FIELD(   myAVCThr, "Thresh.", " dB", -96, 0, 1,    , SetAVCParameters, updateEvent, noStyle)
     , FIELD(   myAVCAtt, "Attack", " dB/s", 0, 10, 1, 0.1, SetAVCParameters, updateEvent, noStyle)
     , FIELD(   myAVCDec,  "Decay", " dB/s", 0, 10, 1, 0.1, SetAVCParameters, updateEvent, noStyle)
    );

TOGGLE(equalizerFlag, setEQ, "Equalizer: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, eqON, noEvent)
       , VALUE("Off", 0, eqOFF, noEvent)
      );

TOGGLE(AVCFlag, setAVC, "Auto Vol Ctl: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, AVCon, noEvent)
       , VALUE("Off", 0, AVCoff, noEvent)
      );

TOGGLE(myInput, selMenu, "Input: ", doNothing, noEvent, wrapStyle
       , VALUE("Mic", AUDIO_INPUT_MIC, SetInput, enterEvent)
       , VALUE("Line", AUDIO_INPUT_LINEIN, SetInput, enterEvent)
      );

MENU(subEQ, "Equalizer cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD(ydBLevel[0], " 150 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[1], " 240 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[2], " 370 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[3], " 600 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[4], " 900 Hz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[5], "1.3 KHz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[6], "2.0 KHz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
     , FIELD(ydBLevel[7], "3.3 KHz", " db", -12, 12, 1, 0.1, EqGainSetL, updateEvent, noStyle)
    );

result toggleAudioSpectrum(eventMask e) {
  if (spectrumFlag == 0) {
    spectrumFlag = 1;
    display.fillScreen(ILI9341_BLACK);
    //display.setFont(Arial_8);
    //display.setTextSize(1);
    display.setTextColor(ILI9341_WHITE); // Draw white text
    display.setCursor(167, 110);
    display.print("-80dB.Peek meter.0dB");
    display.setCursor(15, 125);
    display.print("  0dB");
    display.setCursor(15, 150);
    display.print("-20dB");
    display.setCursor(15, 176);
    display.print("-40dB");
    display.setCursor(15, 200);
    display.print("-60dB");
    display.setCursor(15, 225);
    display.print("-80dB");
    //display.display();

    display.drawRect(50,5,100,60, ILI9341_DARKGREY);
    display.drawRect((320-150),5,100,60, ILI9341_DARKGREY);

    //draw dots for VU displays...
    int xPre = 100;
    int yPre = 60;
    int xPost = 220;
    int yPost = 60;
    int r = 47;

    for (int i=40; i<150; i+=10){
      
      float angleR = i * DEG2RAD;

      int xNeedle = -r * (cos(angleR)) + xPre;
      int yNeedle = -r * (sin(angleR)) + yPre;

      display.drawPixel(xNeedle,yNeedle, ILI9341_WHITE);

      xNeedle = -r * (cos(angleR)) + xPost;
      yNeedle = -r * (sin(angleR)) + yPost;

      display.drawPixel(xNeedle,yNeedle, ILI9341_WHITE);
    }

    display.setCursor(127, 55);
    display.print("Pre");
    display.setCursor(245, 55);
    display.print("Post");

    // Input Settings

    display.fillRect(12,5,30,25, ILI9341_WHITE);
    display.fillRect(12,40,30,25, ILI9341_WHITE);

    if (myInput == AUDIO_INPUT_MIC){
      display.fillRect(13,6,29,24, ILI9341_RED);
    }
    else {
      display.fillRect(13,41,29,24, ILI9341_RED);
    }


    display.setTextColor(ILI9341_BLACK);
    display.setCursor(18, 14);
    display.print("Mic");
    display.setCursor(16, 48);
    display.print("Line");


   // Output Settings

  for (int i=0; i <=8; i++){
    display.fillRect(40*i+2, 75, 35, 20,ILI9341_WHITE);
  }

  // check what's on
  if (noiseGateFlag ==1){
    display.fillRect(3, 76, 34, 19, ILI9341_BLUE); 
  }

  if (procFlag ==1){
    display.fillRect(43, 76, 34, 19, ILI9341_BLUE); 
  }

  if (limFlag ==1){
    display.fillRect(83, 76, 34, 19, ILI9341_BLUE); 
  }

  if (amgFlag ==1){
    display.fillRect(123, 76, 34, 19, ILI9341_BLUE); 
  }
  
  if (mupFlag ==1){
    display.fillRect(163, 76, 34, 19, ILI9341_BLUE); 
  }

  if (equalizerFlag ==1){
    display.fillRect(203, 76, 34, 19, ILI9341_BLUE); 
  }

  if (AVCFlag ==1){
    display.fillRect(243, 76, 34, 19, ILI9341_BLUE); 
  }

    
  display.setTextColor(ILI9341_BLACK);
  display.setCursor(7, 82);
  display.print("Gate");
  display.setCursor(47, 82);
  display.print("Proc");
  display.setCursor(88, 82);
  display.print("Limt");
  display.setCursor(127, 82);
  display.print("A MU");
  display.setCursor(168, 82);
  display.print("MkUp");
  display.setCursor(214, 82);
  display.print("EQ");
  display.setCursor(248, 82);
  display.print("AVol");
    
    //Serial.println();
    //Serial.println("Audio Spectrum ON");
  }
  else {
    spectrumFlag = 0;
    //Serial.println();
    //Serial.println("Audio Spectrum OFF");
  }
  return proceed;
}

result ngON(eventMask e) {
  Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis);
  noiseGateFlag = 1;
  //Serial.println();
  //Serial.print(" myNGattackTime: "); //Serial.print(myNGattackTime); //Serial.print(" myNGreleaseTime: "); //Serial.print(myNGreleaseTime); //Serial.print(" myNGthreshold: "); //Serial.print(myNGthreshold);
  //Serial.print(" myNGholdTime: "); //Serial.print(myNGholdTime, 4); //Serial.print(" myNGhysterisis: "); //Serial.print(myNGhysterisis);
  //Serial.println();
  //Serial.println("Noise Gate ON");
  return proceed;
}

result ngOFF(eventMask e) {
  noiseGateFlag = 0;
  Dynamics.gate( -110.0f, myNGattackTime, myNGreleaseTime, myNGhysterisis);
  //Serial.println();
  //Serial.println("Noise Gate OFF");
  return proceed;
}

MENU(subNG, "Noise Gate cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , altFIELD(decPlaces<3>::menuField,  myNGattackTime,  "Attack", "", 0.0, 1.0, 0.01, 0.001, ngON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myNGreleaseTime, "Release", "", 0.0, 1.0, 0.01, 0.001, ngON, updateEvent, noStyle)
     , FIELD(myNGthreshold, "Thresh.", "", -110.0, 50.0, 1, , ngON, updateEvent, noStyle)
     , FIELD(myNGhysterisis, "Hysteresis", "", 0.0f, 6.0f, 1, 0.1, ngON, updateEvent, noStyle)
     , altFIELD(decPlaces<4>::menuField, myNGholdTime, "Hold", "", 0.0001, 0.01, 0.001, 0.0001, ngON, updateEvent, noStyle)
    );

TOGGLE(noiseGateFlag, setNG, "Noise Gate: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, ngON, noEvent)
       , VALUE("Off", 0, ngOFF, noEvent)
      );

result procON(eventMask e) {
  Dynamics.compression( myPRCthreshold, myPRCattack, myPRCrelease, myPRCratio, myPRCkneeWidth);
  procFlag = 1;
  //Serial.println();
  //Serial.print(" myPRCthreshold: "); //Serial.print(myPRCthreshold); //Serial.print(" myPRCattack: "); //Serial.print(myPRCattack , 4);
  //Serial.print(" myPRCrelease: "); //Serial.print(myPRCrelease, 4); //Serial.print(" myPRCratio: "); //Serial.print(myPRCratio); //Serial.print(" myPRCkneeWidth: "); //Serial.print(myPRCkneeWidth);
  //Serial.println();
  //Serial.println("Processor ON");
  return proceed;
}

result procOFF(eventMask e) {
  Dynamics.compression( 0.0f, 0.03f, 0.5f, 1.0f, 6.0f);
  procFlag = 0;
  //Serial.println();
  //Serial.print(" myPRCthreshold: "); //Serial.print(myPRCthreshold); //Serial.print(" myPRCattack: "); //Serial.print(myPRCattack, 4);
  //Serial.print(" myPRCrelease: "); //Serial.print(myPRCrelease, 4); //Serial.print(" myPRCratio: "); //Serial.print(myPRCratio); //Serial.print(" myPRCkneeWidth: "); //Serial.print(myPRCkneeWidth);
  //Serial.println();
  //Serial.println("Processor OFF");
  return proceed;
}

MENU(subProc, "Processor cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD( myPRCthreshold, "Thresh.", "", -110.0, 0.0, 1, , procON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField,  myPRCattack,  "Attack", "", 0.0, 1.0, 0.01, 0.001, procON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myPRCrelease, "Release", "", 0.0, 1.0, 0.01, 0.001, procON, updateEvent, noStyle)
     , FIELD(     myPRCratio,      "Ratio", "", 1.0, 60.0, 1, , procON, updateEvent, noStyle)
     , FIELD( myPRCkneeWidth, "Knee Width", "", 0.0, 32.0, 1, , procON, updateEvent, noStyle)
    );

TOGGLE(procFlag, setProc, "Processor: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, procON, noEvent)
       , VALUE("Off", 0, procOFF, noEvent)
      );

result limON(eventMask e) {
  Dynamics.limit( myLIMthreshold, myLIMattack, myLIMrelease);
  limFlag = 1;
  //Serial.println();
  //Serial.print(" myLIMthreshold: "); //Serial.print(myLIMthreshold); //Serial.print(" myLIMattack: "); //Serial.print(myLIMattack, 4); //Serial.print(" myLIMrelease: "); //Serial.print(myLIMrelease, 4);
  //Serial.println();
  //Serial.println("Limiter ON");
  return proceed;
}

result limOFF(eventMask e) {
  Dynamics.limit( myLIMthreshold, myLIMattack, myLIMrelease);
  limFlag = 0;
  //Serial.println();
  //Serial.print(" myLIMthreshold: "); //Serial.print(myLIMthreshold); //Serial.print(" myLIMattack: "); //Serial.print(myLIMattack, 4); //Serial.print(" myLIMrelease: "); //Serial.print(myLIMrelease, 4);
  //Serial.println();
  //Serial.println("Limiter OFF");
  return proceed;
}

MENU(subLim, "Limiter cfg", showEvent, anyEvent, wrapStyle
     , EXIT(" <- Back")
     , FIELD( myLIMthreshold, "Thresh.", "", -110.0, 0.0, 1, , limON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField,  myLIMattack,  "Attack", "", 0.0, 4.0, 0.01, 0.001, limON, updateEvent, noStyle)
     , altFIELD(decPlaces<3>::menuField, myLIMrelease, "Release", "", 0.0, 4.0, 0.01, 0.001, limON, updateEvent, noStyle)
    );

TOGGLE(limFlag, setLim, "Limiter: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, limON, noEvent)
       , VALUE("Off", 0, limOFF, noEvent)
      );

MENU(sdCard, "Save Settings", doNothing, noEvent, wrapStyle
     , FIELD(myPreset, "Select preset", "", 0, 9, 1, , doNothing, noEvent, wrapStyle)
     , OP("Read preset", readFromFile, enterEvent)
     , OP("Save preset",  writeToFile, enterEvent)
     , OP("Del. preset",   deleteFile, enterEvent)
     , OP("Load default", resetDefault, enterEvent)
     , EXIT(" <- Back")
    );

result amgON(eventMask e) {
  Dynamics.makeupGain(0.0f);
  mupFlag = 0;
  Dynamics.autoMakeupGain(myAMGheadroom);
  amgFlag = 1;
  //Serial.println();
  //Serial.print(" myAMGheadroom: "); //Serial.print(myAMGheadroom);
  //Serial.println();
  //Serial.println("Auto Makeup Gain ON and Makeup Gain OFF");
  return proceed;
}

result amgOFF(eventMask e) {
  Dynamics.autoMakeupGain(0.0f);
  amgFlag = 0;
  //Serial.println();
  //Serial.print(" myAMGheadroom: "); //Serial.print(0.0f);
  //Serial.println();
  //Serial.println("Auto Makeup Gain OFF");
  return proceed;
}

TOGGLE(amgFlag, setAMG, "Auto MU Gain: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, amgON, noEvent)
       , VALUE("Off", 0, amgOFF, noEvent)
      );

result mupON(eventMask e) {
  Dynamics.autoMakeupGain(0.0f);
  amgFlag = 0;
  Dynamics.makeupGain(myMUPgain);
  mupFlag = 1;
  //Serial.println();
  //Serial.print(" myMUPgain: "); //Serial.print(myMUPgain);
  //Serial.println();
  //Serial.println("Makeup Gain ON and Auto Makeup OFF");
  return proceed;
}

result mupOFF(eventMask e) {
  Dynamics.makeupGain(0.0f);
  mupFlag = 0;
  //Serial.println();
  //Serial.print(" myMUPgain: "); //Serial.print(0.0f);
  //Serial.println();
  //Serial.println("Makeup Gain OFF");
  return proceed;
}

TOGGLE(mupFlag, setMUP, "Makeup Gain: ", doNothing, noEvent, wrapStyle
       , VALUE("On", 1, mupON, noEvent)
       , VALUE("Off", 0, mupOFF, noEvent)
      );

MENU(mainMenu, "Main menu", doNothing, noEvent, wrapStyle
     , OP("Spectrum disp", toggleAudioSpectrum, enterEvent)
     , SUBMENU(subLevels)
     , SUBMENU(subNG)
     , SUBMENU(setNG)
     , SUBMENU(subProc)
     , SUBMENU(setProc)
     , SUBMENU(subLim)
     , SUBMENU(setLim)
     , SUBMENU(setAMG)
     , SUBMENU(setMUP)
     , SUBMENU(subEQ)
     , SUBMENU(setEQ)
     , SUBMENU(subAVC)
     , SUBMENU(setAVC)
     , SUBMENU(selMenu)
     , SUBMENU(sdCard)
    );

result showEvent(eventMask e, navNode & nav, prompt & item) {
  //Serial.println();
  //Serial.print("event: ");
  //Serial.print(e);
  return proceed;
}

// Configuration https://github.com/neu-rah/ArduinoMenu/wiki/Menu-definition

#define MAX_DEPTH 2

// define output device
idx_t serialTops[MAX_DEPTH] = {0};
serialOut outSerial(Serial, serialTops);

MENU_OUTPUTS(out,MAX_DEPTH
  ,ADAGFX_OUT(display,my_colors,6,13,{1,0,UC_Width/6,UC_Height/10})
  ,SERIAL_OUT(Serial)
);

//serialIn serial(Serial);

//macro to create navigation control root object (nav) using mainMenu
NAVROOT(nav, mainMenu, MAX_DEPTH, in, out);
//NAVROOT(nav, mainMenu, MAX_DEPTH, serial, out);

result doAlert(eventMask e, prompt & item) {
  nav.idleOn(alert);
  return proceed;
}

//when menu is suspended
result idle(menuOut & o, idleEvent e) {
  //display.screenClear();
  //if (&display == &display) {
  //  if (e == idling) {
  //    //Serial.println("OLED");
  //    //Serial.println("Suspended menu");
  //  }
  //} else
  //  switch (e) {
  //    case idleStart: //Serial.println("suspending menu!"); break;
  //    case idling: //Serial.println("suspended..."); break;
  //    case idleEnd: //Serial.println("resuming menu."); break;
  //  }
  return proceed;
}

void MyDelay(unsigned long ms)
{
  unsigned long currentMillis = millis();
  unsigned long previousMillis = millis();

  while (currentMillis - previousMillis < ms) {
    currentMillis = millis();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void setup(void) {
  pinMode(2, INPUT);
  pinMode(encBtn, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  display.begin();
  //display.setFont(Arial_8);

  display.fillScreen(ILI9341_BLACK);
  display.setRotation(1);
  display.setCursor(0, 0);
  display.print("VA3HDL - Aurora,ON");
  display.setCursor(0, 20);
  display.print("Version 5.0");
  display.setCursor(0, 40);
  Timer1.initialize(500);
  Timer1.attachInterrupt(timerIsr);
 
  AudioMemory(100);
  audioShield.enable();
   
  readFromFile();  //  Check for and restore last save state if present

  delay(2000);
  display.fillScreen(ILI9341_BLACK);
  display.setCursor(0, 100);
  display.fillScreen(ILI9341_BLACK);

  nav.idleTask = idle; //point a function to be used when menu is suspended
  nav.useUpdateEvent = true;
  //Serial.println("Here2");
}

elapsedMillis fps;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  ////Serial.println("Loop");
  if (spectrumFlag != 1){
    nav.poll();
  }
  if (spectrumFlag == 1) {
    ////Serial.println("Loop2");
    displayAudioSpectrum();
    b = clickEncoder.getButton();
    if (b == ClickEncoder::Clicked) {
      //Serial.println("Clicked");
      spectrumFlag = 0;
      display.fillScreen(ILI9341_BLACK);
      //display.display();
      nav.refresh();
      //Serial.println();
      //Serial.println("Exiting Spectrum");
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SetAudioShield() {
  audioShield.muteHeadphone();
  audioShield.muteLineout();

  audioShield.inputSelect(myInput);
  audioShield.micGain(micGainSet);

  audioShield.lineInLevel(myLineInLevel);
  audioShield.lineOutLevel(myLineOutLevel);

  audioShield.volume(myVolume);
  audioShield.adcHighPassFilterEnable();
  audioShield.dacVolume(1);
  audioShield.dacVolumeRamp();

  audioShield.audioPreProcessorEnable();
  audioShield.audioPostProcessorEnable();

  if (equalizerFlag == 1) eqON(evt); else eqOFF(evt);

  SetupFilters();         // Setup Equalizer bands
  EqGainSetL();           // Setup Equalizer levels
  SetAVCParameters();     // Setup AVC parameters

  /* Dynamics - Order of events:
    1. gate -> timeToAlpha -> timeToAlpha
    2. compression -> computeMakeupGain -> timeToAlpha -> timeToAlpha
    3. limit -> computeMakeupGain -> timeToAlpha -> timeToAlpha
    4. makeupgain
    5. autoMakeupGain -> computeMakeupGain
  */

  if (noiseGateFlag == 1) ngON(evt); else ngOFF(evt);

  if (procFlag == 1) procON(evt); else procOFF(evt);

  if (limFlag == 1) limON(evt); else limOFF(evt);

  if (mupFlag == 1) mupON(evt); else mupOFF(evt);

  if (amgFlag == 1) amgON(evt); else amgOFF(evt);

  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
}

void SetAVCParameters() {
  //Serial.println();
  //Serial.print(" Comp Gain: "); //Serial.print(myAVCGain); //Serial.print(" Comp Resp: "); //Serial.print(myAVCResp); //Serial.print(" Hardlimit: "); //Serial.print(myAVCHard);
  //Serial.print(" Threshold: "); //Serial.print( myAVCThr); //Serial.print(" Attack: "); //Serial.print(myAVCAtt); //Serial.print(" Decay: "); //Serial.print(myAVCDec);
  //Serial.print(" Mic Gain: "); //Serial.print(micGainSet);

  if (AVCFlag == 1) {
    audioShield.autoVolumeControl( myAVCGain    // Maximum gain that can be applied 0 - 0 dB / 1 - 6.0 dB / 2 - 12 dB
                                   , myAVCResp  // Integrator Response 0 - 0 mS / 1 - 25 mS / 2 - 50 mS / 3 - 100 mS
                                   , myAVCHard  // hardLimit
                                   , myAVCThr   // threshold floating point in range 0 to -96 dB
                                   , myAVCAtt   // attack floating point figure is dB/s rate at which gain is increased
                                   , myAVCDec); // decay floating point figure is dB/s rate at which gain is reduced
    audioShield.autoVolumeEnable();
  }
}

void SetLevels() {
  //Serial.println();
  //Serial.print(" Headphone: "); //Serial.print(myVolume); //Serial.print(" Line In Level: "); //Serial.print(myLineInLevel); //Serial.print(" Line Out Level: "); //Serial.print(myLineOutLevel);
  //Serial.print(" myAMGheadroom: "); //Serial.print(myAMGheadroom); //Serial.print(" myMUPgain: "); //Serial.print(myMUPgain);
  audioShield.volume(myVolume);
  audioShield.micGain(micGainSet);
  audioShield.lineInLevel(myLineInLevel);
  audioShield.lineOutLevel(myLineOutLevel);
  if (amgFlag == 1) Dynamics.autoMakeupGain(myAMGheadroom);
  if (mupFlag == 1) Dynamics.makeupGain(myMUPgain);
}

void SetInput() {
  if (myInput == AUDIO_INPUT_MIC)
  {
    audioShield.lineInLevel(0);
    audioShield.micGain(micGainSet);
  }
  else
  {
    audioShield.lineInLevel(myLineInLevel);
    audioShield.micGain(0);
  }
  SetAudioShield();
  //Serial.println(); //Serial.print("Input: "); //Serial.print(myInput);
}

void EqGainSetL() {
  for (int freqBand = 0; freqBand < 8; freqBand++) {
    bandGain[freqBand] = pow(10, (ydBLevel[freqBand] / 20));
    if (ydBLevel[freqBand] <= -12) {
      bandGain[freqBand] = 0;
    }
    //Serial.println();
    //Serial.print("Eq. band: "); //Serial.print(freqBand); //Serial.print(" dB: "); //Serial.print(ydBLevel[freqBand]); //Serial.print(" Gain: "); //Serial.print(bandGain[freqBand]);
  }

  EQ_Mix1.gain(0, bandGain[0]);     //  140
  EQ_Mix1.gain(1, bandGain[1]);     //  240
  EQ_Mix1.gain(2, bandGain[2]);     //  370
  EQ_Mix1.gain(3, bandGain[3]);     //  590

  EQ_Mix2.gain(0, bandGain[4]);     //  900
  EQ_Mix2.gain(1, bandGain[5]);     //  1300
  EQ_Mix2.gain(2, bandGain[6]);     //  2000
  EQ_Mix2.gain(3, bandGain[7]);     //  3300

  EQ_MixOut.gain(0, 1);  // Combine Mixers
  EQ_MixOut.gain(1, 1);
  EQ_MixOut.gain(2, 0);
}

void SetupFilters() {
  EQ_1.setBandpass(0, eqFreq[0], .9);  // Band 1 140Hz, Q =.9
  EQ_1.setBandpass(1, eqFreq[0], .9);
  EQ_1.setBandpass(2, eqFreq[0], .9);
  EQ_1.setBandpass(3, eqFreq[0], .9);

  EQ_2.setBandpass(0, eqFreq[1], .9);  // Band 2 240Hz
  EQ_2.setBandpass(1, eqFreq[1], .9);
  EQ_2.setBandpass(2, eqFreq[1], .9);
  EQ_2.setBandpass(3, eqFreq[1], .9);

  EQ_3.setBandpass(0, eqFreq[2], .9);  // Band 3 370Hz
  EQ_3.setBandpass(1, eqFreq[2], .9);
  EQ_3.setBandpass(2, eqFreq[2], .9);
  EQ_3.setBandpass(3, eqFreq[2], .9);

  EQ_4.setBandpass(0, eqFreq[3], .9);  // Band 4 590Hz
  EQ_4.setBandpass(1, eqFreq[3], .9);
  EQ_4.setBandpass(2, eqFreq[3], .9);
  EQ_4.setBandpass(3, eqFreq[3], .9);

  EQ_5.setBandpass(0, eqFreq[4], .9);  // Band 5 900Hz
  EQ_5.setBandpass(1, eqFreq[4], .9);
  EQ_5.setBandpass(2, eqFreq[4], .9);
  EQ_5.setBandpass(3, eqFreq[4], .9);

  EQ_6.setBandpass(0, eqFreq[5], .9);  // Band 6 1300Hz
  EQ_6.setBandpass(1, eqFreq[5], .9);
  EQ_6.setBandpass(2, eqFreq[5], .9);
  EQ_6.setBandpass(3, eqFreq[5], .9);

  EQ_7.setBandpass(0, eqFreq[6], .9);  // Band 7 2000Hz
  EQ_7.setBandpass(1, eqFreq[6], .9);
  EQ_7.setBandpass(2, eqFreq[6], .9);
  EQ_7.setBandpass(3, eqFreq[6], .9);

  EQ_8.setBandpass(0, eqFreq[7], .9);  // Band 8 3300 Hz
  EQ_8.setBandpass(1, eqFreq[7], .9);
  EQ_8.setBandpass(2, eqFreq[7], .9);
  EQ_8.setBandpass(3, eqFreq[7], .9);
}

void displayAudioSpectrum() {
  const int nBars = sizeof(fftOctTab) / 2 ;
  const int barWidth = 12;
  const int posX = 55;
  const int posY = 230;
  const int minHeight = 1;
  const int maxHeight = 25;
  const int barLen = 10;
  const int barGap = 1;
  static uint16_t bar = 0;

  float n;
  int16_t val;


  float peakFloat = 1.2;
  float peak;
  int peakPreM = 0;
  int peakPostM = 0;
  int mVal = 0;

  if (fps > 75) { 
    fps = 0;
    
    if (peakPre.available()) {
      peak = peakPre.read();
      peakPreM = map(peak, 0.0, 1.0, 40, 140);
      //display.drawFastHLine(0, 12, 180, ILI9341_BLACK);
      //display.drawFastHLine(0, 12, peakM, ILI9341_WHITE);
    }
    if (peakPost.available()) {
      peak = peakPost.read();
      peakPostM = map(peak, 0.0, 1.0, 40, 140);
      //display.drawFastHLine(0, 15, 180, ILI9341_BLACK);
      //display.drawFastHLine(0, 15, peakM, ILI9341_WHITE);
    }    

    //angle is 0-128
    int xPre = 100;
    int yPre = 60;
    int rPre = 45;
    float needleReaction = .25;
    float peakPreR = peakPreM * DEG2RAD;
    peakPreR = ((peakPreR - peakPreROld) * needleReaction)+ peakPreROld;
    peakPreROld = peakPreR;

    int xPreNeedle = -rPre * (cos(peakPreR)) + xPre;
    int yPreNeedle = -rPre * (sin(peakPreR)) + yPre;

    display.drawLine(xPre,yPre, xPreNeedleOld,yPreNeedleOld, ILI9341_BLACK);
    display.drawLine(xPre,yPre, xPreNeedle,yPreNeedle, ILI9341_RED);
    xPreNeedleOld=xPreNeedle;
    yPreNeedleOld=yPreNeedle;

    //angle is 0-180
    int xPost = 220;
    int yPost = 60;
    int rPost = 45;

    float peakPostR = peakPostM * DEG2RAD;
    peakPostR = ((peakPostR - peakPostROld) * needleReaction)+ peakPostROld;
    peakPostROld = peakPostR;

    int xPostNeedle = -rPost * (cos(peakPostR)) + xPost;
    int yPostNeedle = -rPost * (sin(peakPostR)) + yPost;

    display.drawLine(xPost,yPost, xPostNeedleOld,yPostNeedleOld, ILI9341_BLACK);
    display.drawLine(xPost,yPost, xPostNeedle,yPostNeedle, ILI9341_RED);
    xPostNeedleOld=xPostNeedle;
    yPostNeedleOld=yPostNeedle;
    
    if (fftValues.available()) {
      for (int i=0;i < nBars; i++) {
         n = fftValues.read(fftOctTab[i * 2], fftOctTab[i * 2 + 1]);
         val = log10f(n) * 60 + 252;
         if (val > maxVal) maxVal = val;
         mVal = map(val, 0, maxVal, minHeight, maxHeight);        
         int x = posX + i * barWidth;
         
         if (mVal >= barPeak[i]){
          barPeak[i]=mVal;
         }
         else{
          barPeak[i] = barPeak[i] - peakFloat;
         }
         for (int barNum=0; barNum < maxHeight; barNum ++){
          if (barNum < mVal){
            display.drawFastHLine(x, posY-(barNum*4), barLen, ILI9341_GREEN);           
          }
          else {
            display.drawFastHLine(x, posY-(barNum*4), barLen, ILI9341_BLACK);
          }

          if (barNum == round(barPeak[i])){
            display.drawFastHLine(x, posY-(barNum*4), barLen, ILI9341_WHITE);           
          }

        }
      }
    }
  }
}

void readFromFile()
{
  boolean they_match = true;
  
  EEPROM.get(0,tempMagic);
  for ( int i = 0; i < 4; i++ ) 
  {
     if ( tempMagic[i] != EEPROM_MAGIC[i] ) 
     {
        they_match = false;
        break;
     }
  }
  if(they_match) {
     int address = sizeof(EEPROM_MAGIC);
     EEPROM.get(address,mySaveSet);
     AVCgain = mySaveSet.AVCgain;     
     AVCFlag = mySaveSet.AVCFlag;
     equalizerFlag = mySaveSet.equalizerFlag;
     myLineInLevel = mySaveSet.myLineInLevel;
     myLineOutLevel = mySaveSet.myLineOutLevel;
     myVolume = mySaveSet.myVolume;
     micGainSet = mySaveSet.micGainSet;
     myAVCGain = mySaveSet.myAVCGain;
     myAVCResp = mySaveSet.myAVCResp;
     myAVCHard = mySaveSet.myAVCHard;
     myAVCThr = mySaveSet.myAVCThr;
     myAVCAtt = mySaveSet.myAVCAtt;
     myAVCDec = mySaveSet.myAVCDec;
     myInput = mySaveSet.myInput;
     ydBLevel[0] = mySaveSet.ydBLevel[0];
     ydBLevel[1] = mySaveSet.ydBLevel[1];
     ydBLevel[2] = mySaveSet.ydBLevel[2];
     ydBLevel[3] = mySaveSet.ydBLevel[3];
     ydBLevel[4] = mySaveSet.ydBLevel[4];
     ydBLevel[5] = mySaveSet.ydBLevel[5];
     ydBLevel[6] = mySaveSet.ydBLevel[6];
     ydBLevel[7] = mySaveSet.ydBLevel[7];     
     myNGattackTime = mySaveSet.myNGattackTime;
     myNGreleaseTime = mySaveSet.myNGreleaseTime;
     myNGthreshold = mySaveSet.myNGthreshold;
     myNGholdTime = mySaveSet.myNGholdTime;
     noiseGateFlag = mySaveSet.noiseGateFlag;
     myNGhysterisis = mySaveSet.myNGhysterisis;
     procFlag = mySaveSet.procFlag;
     myPRCthreshold = mySaveSet.myPRCthreshold;
     myPRCattack = mySaveSet.myPRCattack;
     myPRCrelease = mySaveSet.myPRCrelease;
     myPRCratio = mySaveSet.myPRCratio;
     myPRCkneeWidth = mySaveSet.myPRCkneeWidth;
     limFlag = mySaveSet.limFlag;
     myLIMthreshold = mySaveSet.myLIMthreshold;
     myLIMattack = mySaveSet.myLIMattack;
     myLIMrelease = mySaveSet.myLIMrelease;
     amgFlag = mySaveSet.amgFlag;
     myAMGheadroom = mySaveSet.myAMGheadroom;
     mupFlag = mySaveSet.mupFlag;
     myMUPgain = mySaveSet.myMUPgain;        

     display.fillScreen(ILI9341_BLACK);
     display.setCursor(0, 0);
     display.print("Settings read");
  } else {
     display.fillScreen(ILI9341_BLACK);
     display.setCursor(0, 0);
     display.print("No settings found");
  }
  SetAudioShield();
}

void writeToFile()
{
   mySaveSet.AVCgain = AVCgain;     
   mySaveSet.AVCFlag = AVCFlag;
   mySaveSet.equalizerFlag = equalizerFlag;
   mySaveSet.myLineInLevel = myLineInLevel;
   mySaveSet.myLineOutLevel = myLineOutLevel;
   mySaveSet.myVolume = myVolume;
   mySaveSet.micGainSet = micGainSet;
   mySaveSet.myAVCGain = myAVCGain;
   mySaveSet.myAVCResp = myAVCResp;
   mySaveSet.myAVCHard = myAVCHard;
   mySaveSet.myAVCThr = myAVCThr;
   mySaveSet.myAVCAtt = myAVCAtt;
   mySaveSet.myAVCDec = myAVCDec;
   mySaveSet.myInput = myInput;
   mySaveSet.ydBLevel[0] = ydBLevel[0];
   mySaveSet.ydBLevel[1] = ydBLevel[1];
   mySaveSet.ydBLevel[2] = ydBLevel[2];
   mySaveSet.ydBLevel[3] = ydBLevel[3];
   mySaveSet.ydBLevel[4] = ydBLevel[4];
   mySaveSet.ydBLevel[5] = ydBLevel[5];
   mySaveSet.ydBLevel[6] = ydBLevel[6];
   mySaveSet.ydBLevel[7] = ydBLevel[7];     
   mySaveSet.myNGattackTime = myNGattackTime;
   mySaveSet.myNGreleaseTime = myNGreleaseTime;
   mySaveSet.myNGthreshold = myNGthreshold;
   mySaveSet.myNGholdTime = myNGholdTime;
   mySaveSet.noiseGateFlag= noiseGateFlag;
   mySaveSet.myNGhysterisis = myNGhysterisis;
   mySaveSet.procFlag = procFlag;
   mySaveSet.myPRCthreshold = myPRCthreshold;
   mySaveSet.myPRCattack = myPRCattack;
   mySaveSet.myPRCrelease = myPRCrelease;
   mySaveSet.myPRCratio = myPRCratio;
   mySaveSet.myPRCkneeWidth = myPRCkneeWidth;
   mySaveSet.limFlag = limFlag;
   mySaveSet.myLIMthreshold = myLIMthreshold;
   mySaveSet.myLIMattack = myLIMattack;
   mySaveSet.myLIMrelease = myLIMrelease;
   mySaveSet.amgFlag = amgFlag;
   mySaveSet.myAMGheadroom = myAMGheadroom;
   mySaveSet.mupFlag = mupFlag;
   mySaveSet.myMUPgain = myMUPgain;        

   EEPROM.put(0,EEPROM_MAGIC);
   int address = sizeof(EEPROM_MAGIC);
   EEPROM.put(address,mySaveSet);

   display.fillScreen(ILI9341_BLACK);
   display.setCursor(0, 0);
   display.print("Settings saved");
}

void deleteFile()
{
   EEPROM.put(0,EEPROM_INIT);
}

void resetDefault(void)
{
  AVCgain = AVCGAIN;
  AVCFlag = AVCFLAG;

  myAVCGain = MYAVCGAIN;
  myAVCResp = MYAVCRESP;
  myAVCHard = MYAVCHARD;
  myAVCThr = MYAVCTHR;
  myAVCAtt = MYAVCATT;
  myAVCDec = MYAVCDEC;

  bandGain[0] = 1;
  bandGain[1] = 1;
  bandGain[2] = 1;
  bandGain[3] = 1;
  bandGain[4] = 1;
  bandGain[5] = 1;
  bandGain[6] = 1;
  bandGain[7] = 1;

  ydBLevel[0] = 0;
  ydBLevel[1] = 0;
  ydBLevel[2] = 0;
  ydBLevel[3] = 0;
  ydBLevel[4] = 0;
  ydBLevel[5] = 0;
  ydBLevel[6] = 0;
  ydBLevel[7] = 0;

  equalizerFlag = EQUALIZERFLAG;
  noiseGateFlag = NOISEGATEFLAG;
  myLineInLevel = MYLINEINLEVEL;
  myLineOutLevel = MYLINEOUTLEVEL;
  myVolume = MYVOLUME;
  micGainSet = MICGAINSET;
  myInput = MYINPUT;

  // Default Noise Gate parameters
  myNGattackTime = MYNGATTACKTIME;
  myNGreleaseTime = MYNGRELEASETIME;
  myNGthreshold = MYNGTHRESHOLD;
  myNGhysterisis = MYNGHYSTERISIS;
  myNGholdTime = MYNGHOLDTIME;

  // Default Processor parameters
  procFlag = PROCFLAG;
  myPRCthreshold = MYPRCTHRESHOLD;
  myPRCattack = MYPRCATTACK;
  myPRCrelease = MYPRCRELEASE;
  myPRCratio = MYPRCRATIO;
  myPRCkneeWidth = MYPRCKNEEWIDTH;

  // Default Limiter parameters
  limFlag = LIMFLAG;
  myLIMthreshold = MYLIMTHRESHOLD;
  myLIMattack = MYLIMATTACK;
  myLIMrelease = MYLIMRELEASE;

  // Default Auto Makeup Gain parameters
  amgFlag = AMGFLAG;
  myAMGheadroom = MYAMGHEADROOM;

  // Default Makeup Gain parameters
  mupFlag = MUPFLAG;
  myMUPgain = MYMUPGAIN;
  
  SetAudioShield();
  
  display.fillScreen(ILI9341_BLACK);
  display.setCursor(0, 0);
  display.print("Defaults loaded");
}
