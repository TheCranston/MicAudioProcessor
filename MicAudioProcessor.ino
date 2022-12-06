#include "OpenAudio_ArduinoLibrary.h"  // https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include "AudioStream_F32.h"
#include "USB_Audio_F32.h"        // But why not?!?!?
#include "effect_dynamics_F32.h"  // Add this: https://github.com/MarkzP/AudioEffectDynamics_F32

// Not sure why I had to do this, they are included in the line previous....
#ifndef MIN_DB
#define MIN_DB -110.0f
#endif
#ifndef MAX_DB
#define MAX_DB 0.0f
#endif

#include "Audio.h"
#include <EEPROM.h>  // store the config here for power persistance
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <ILI9341_T4.h>  // the screen driver library : https://github.com/vindar/ILI9341_T4
#include <tgx.h>         // the tgx library
#include <font_tgx_OpenSans.h>
#include <Image.h>
#include <elapsedMillis.h>

// Load sprites
#include "DSPBKGRND.h"
#include "LITGREEN.h"
#include "LITWHITE.h"
#include "LITGREENVU.h"
#include "LITWHITEVU.h"
#include "EQSlider.h"
#include "Mic_On.h"
#include "Gate_On.h"
#include "Comp_On.h"
#include "Limit_On.h"
#include "EQ_On.h"
#include "A-VolOn.h"
#include "A-MuOn.h"
#include "MkUpOn.h"
#include "Mixer.h"
#include "Gate2.h"
#include "GateOn.h"
#include "ThresholdOn.h"
#include "ReleaseOn.h"
#include "AttackOn.h"
#include "HysterisisOn.h"
#include "Comp.h"
#include "CompOn.h"



#include <TimerOne.h>
#include <ClickEncoder.h>

const float DEG2RAD = PI / 180.0f;
const float RAD2DEG = 180.0f / PI;

// namespace for draw graphics primitives
using namespace tgx;

// Define your font here. Default font: lcd5x7
//#define menuFont X11fixed7x14
#define fontW 7
#define fontH 14

// Encoder /////////////////////////////////////
#define encA 3
#define encB 4
#define encBtn 2
ClickEncoder clickEncoder(encA, encB, encBtn, 2);

#define enc2A 17  // DT
#define enc2B 16  // CLK
#define enc2Btn 5
ClickEncoder clickEncoder2(enc2A, enc2B, enc2Btn, 2);

// ClickEncoderStream encStream(clickEncoder, 1);
// MENU_INPUTS(in, &encStream);

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

void timerIsr() {
  clickEncoder.service();
  clickEncoder2.service();
}

// Let's get programmatic about our settings and not take the defaults, eh?
const float sample_rate_Hz = 44100.0f;  // overboard for our appliation, but why not?
const int audio_block_samples = 128;    // max numb of blocks, and the default
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);

AudioControlSGTL5000 audioShield;

AudioInputI2S_F32 audioInput(audio_settings);
AudioInputUSB_F32 audioInUSB1(audio_settings);
AudioOutputI2S_F32 audioOutput(audio_settings);

AudioMixer4_F32 inputMixer(audio_settings);
AudioAnalyzePeak_F32 peakPre;
AudioSwitch4_OA_F32 eqSwitch;
AudioFilterEqualizer_F32 equalize(audio_settings);  //https://forum.pjrc.com/threads/60928-Audio-Equalizer-using-FIR
AudioMixer4_F32 EQ_mix(audio_settings);
AudioEffectDynamics_F32 Dynamics(audio_settings, AudioEffectDynamics_F32::DetectorType_RMS);  //Switch from Peak detect to RMS
AudioAnalyzePeak_F32 peakPost;
AudioAnalyzeFFT1024_F32 fftValues;

AudioConnection_F32 patchCord1(audioInput, 0, inputMixer, 0);
AudioConnection_F32 patchCord2(audioInUSB1, 0, inputMixer, 1);
AudioConnection_F32 patchCord3(inputMixer, peakPre);
AudioConnection_F32 patchCord4(inputMixer, eqSwitch);
AudioConnection_F32 patchCord5(eqSwitch, 0, equalize, 0);
AudioConnection_F32 patchCord7(eqSwitch, 1, EQ_mix, 2);
AudioConnection_F32 patchCord8(equalize, 0, EQ_mix, 0);
AudioConnection_F32 patchCord10(EQ_mix, Dynamics);
AudioConnection_F32 patchCord11(Dynamics, fftValues);
AudioConnection_F32 patchCord12(Dynamics, 0, audioOutput, 0);
AudioConnection_F32 patchCord13(Dynamics, peakPost);

// DEFAULT FLAGS
#define AVCFLAG 0;
#define EQUALIZERFLAG 0;
#define NOISEGATEFLAG 0;
#define MYINPUT AUDIO_INPUT_LINEIN;

// DEFAULT AVC LEVELS
#define AVCGAIN 1.0f;
#define MYAVCGAIN 0;
#define MYAVCRESP 1;
#define MYAVCHARD 0;
#define MYAVCTHR -18.0f;
#define MYAVCATT 0.5f;
#define MYAVCDEC 1.0f;

// DEFAULT LEVELS
#define MYLINEINLEVEL 0;
#define MYLINEOUTLEVEL 29;
#define MYVOLUME 0.7f;
#define MICGAINSET 35;

// DEFAULT NOISE GATE PARAMETERS
#define MYNGATTACKTIME 0.01f;
#define MYNGRELEASETIME 0.2f;
#define MYNGTHRESHOLD -65.0f;
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

// Quick Menu
int eqQMOffset = 40;
int currentQuickMenuSelection = 0;
int currentQuickMenuLevel = 0;
int quickMenuBox[22][4] = {
  { 6, 19, 22, 66 },               // IN Mic Gain
  { 26, 19, 22, 66 },              // IN LINE Gain
  { 31+eqQMOffset, 6, 24, 79 },    // EQ 1
  { 53+eqQMOffset, 6, 24, 79 },    // EQ 2
  { 75+eqQMOffset, 6, 24, 79 },    // EQ 3
  { 98+eqQMOffset, 6, 24, 79 },    // EQ 4
  { 120+eqQMOffset, 6, 24, 79 },   // EQ 5
  { 142+eqQMOffset, 6, 24, 79 },   // EQ 6
  { 164+eqQMOffset, 6, 24, 79 },   // EQ 7
  { 187+eqQMOffset, 6, 24, 79 },   // EQ 8
  { 272, 19, 22, 66 },             // OUT Phone Gain
  { 292, 19, 22, 66 },             // OUTLINE Gain
  {  1,  94, 15, 22 },   // Secret Mixer Menu 
  { 14,  94, 36, 22 },   // Line
  { 50,  94, 36, 22 },   // GAte
  { 87,  94, 36, 22 },   // Comp
  { 124, 94, 36, 22 },  // LImit
  { 161, 94, 36, 22 },  // EQ
  { 198, 94, 37, 22 },  // A-Vol
  { 235, 94, 37, 22 },  // A-Mu
  { 272, 94, 37, 22 }   // MkUp
};

const int QUICK_MENU_SELECTIONS = 20;

// Mixer Menu
int currentMixerMenuSelection = 99;
int mixerMenuBox[6][4] = {
  { 33, 16, 24, 68 },   // IN Gain
  { 58, 16, 24, 68 },   // EQ 1
  { 94, 8, 26, 75 },    // EQ 2
  { 132, 15, 24, 68 },  // EQ 3
  { 155, 15, 26, 68 },  // EQ 3
  { 180, 15, 24, 68 }   // EQ 4
};
const int MIXER_MENU_SELECTIONS = 5;

// Gate Menu
int currentGateMenuSelection = 0;
const int GATE_MENU_SELECTIONS = 4;
int gateMenuBox[6][4] = {
  { 32,  72, 16, 14 },   // Gate ON
  { 53,  72, 30, 14 },   // Threshold
  { 90,  72, 30, 14 },   // Attack 
  { 129, 72, 30, 14 },   // Release 
  { 170, 72, 30, 14 }    // Hysterisis
};

// Compressor Menu
int compPre = 0; // persistent VU display 
int currentCompMenuSelection = 0;
const int COMP_MENU_SELECTIONS = 5;
int compMenuBox[6][4] = {
  { 34,  68, 16, 14 },   // Comp ON
  { 39,  10, 33, 15 },   // Threshold
  { 84,  10, 33, 15 },   // Ratio
  { 39,  27, 33, 15 },   // Attack
  { 84,  27, 33, 15 },   // Release
  { 84,  54, 33, 15 }    // Knee
};



elapsedMillis fps;

// EEPROM Magic - if present then we have a config to load
int EEPROM_MAGIC[4] = { 0xFE, 0xED, 0xC0, 0xDE };  // https://en.wikipedia.org/wiki/Hexspeak for the LOLs
int EEPROM_INIT[4] = { 0x00, 0x00, 0x00, 0x00 };   // invalid config
int tempMagic[4] = { 0x00, 0x00, 0x00, 0x00 };

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
int c;
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
//const int eqFreq[] = {150, 240, 370, 590, 900, 1300, 2000, 3300};
float32_t fBand[] = { 150.0, 240.0, 370.0, 590.0, 900.0, 1300.0, 2000.0, 3300.0 };
float32_t dbBand[] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
float32_t equalizeCoeffs[200];

float bandGain[] = { 1, 1, 1, 1, 1, 1, 1, 1 };
float ydBLevel[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

int equalizerFlag = EQUALIZERFLAG;
int noiseGateFlag = NOISEGATEFLAG;
int myLineInLevel = MYLINEINLEVEL;    // range is 0 to 15
int myLineOutLevel = MYLINEOUTLEVEL;  // range is 13 to 31
float myVolume = MYVOLUME;            // 0.8 is max for undistorted headphone output
float micGainSet = MICGAINSET;        // 35 for the Chinese MH-1B8 mic
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

// Added vars for mixer
float myUSBInputFader = 0.5;  // Range 0 to 1f  - 0 = USB, 1 = INPUT
float myDACVolume = 1.0;      // dac volume 0 to 1f

float barPeak[20];
float xPreNeedleOld;
float yPreNeedleOld;
float xPostNeedleOld;
float yPostNeedleOld;
float peakPreROld;
float peakPostROld;

const uint16_t fftOctTab[] = {
  1, 1,
  2, 2,
  3, 3,
  4, 5,
  6, 8,
  9, 11,
  12, 15,
  16, 21,
  22, 28,
  29, 37,
  38, 49,
  50, 64,
  65, 84,
  85, 110,
  111, 143,
  144, 185,
  186, 239,
  240, 308,
  309, 397,
  398, 511
};
/**  
1, 1,
1, 2,
2, 3,
3, 4,
4, 6,
6, 8,
8, 12,
12, 16,
16, 22,
22, 30,
30, 42,
42, 57,
57, 78,
78, 107,
107, 147,
147, 200,
200, 274,
274, 374,
374, 511
}; 
**/
// Bands calculated from https://forum.pjrc.com/threads/32677-Is-there-a-logarithmic-function-for-FFT-bin-selection-for-any-given-of-bands

// 40MHz SPI. Can do much better with short wires
#define SPI_SPEED 30000000

// screen size in portrait mode
#define LX 320
#define LY 240

// 2 diff buffers with about 8K memory each
DMAMEM ILI9341_T4::DiffBuffStatic<8000> diff1;
DMAMEM ILI9341_T4::DiffBuffStatic<8000> diff2;

// the internal framebuffer for the ILI9341_T4 driver (150KB)
// in DMAMEM to save space in the lower (faster) part of RAM.
DMAMEM uint16_t ib[LX * LY];  // used for internal buffering
DMAMEM uint16_t fb[LX * LY];  // paint in this buffer

// image that encapsulates fb.
Image<RGB565> im(fb, LX, LY);

//
// DEFAULT WIRING USING SPI 0 ON TEENSY 4/4.1
//
#define PIN_SCK 13   // mandatory
#define PIN_MISO 12  // mandatory
#define PIN_MOSI 11  // mandatory
#define PIN_DC 10    // 10 mandatory, can be any pin but using pin 10 (or 36 or 37 on T4.1) provides greater performance

#define PIN_CS 9          //  9 optional (but recommended), can be any pin.
#define PIN_RESET 22      // 22 optional (but recommended), can be any pin.
#define PIN_BACKLIGHT 1   //  1 optional, set this only if the screen LED pin is connected directly to the Teensy.
#define PIN_TOUCH_IRQ 14  // 14 optional. set this only if the touchscreen is connected on the same SPI bus
#define PIN_TOUCH_CS 6    //  6 optional. set this only if the touchscreen is connected on the same spi bus

// Setting the screen driver for the Spectrum Display
// the screen driver object
ILI9341_T4::ILI9341Driver display(PIN_CS, PIN_DC, PIN_SCK, PIN_MOSI, PIN_MISO, PIN_RESET, PIN_TOUCH_CS, PIN_TOUCH_IRQ);

// Equalizer on
bool eqON() {
  equalizerFlag = 1;
  eqSwitch.setChannel(0);
  return true;
}

// Equalizer off
bool eqOFF() {
  equalizerFlag = 0;
  eqSwitch.setChannel(1);
  return true;
}

bool AVCon() {
  // Disabled for now
  AVCFlag = 1;
  return true;
}

// AVC off
bool AVCoff() {
  // Disabled for now
  AVCFlag = 0;
  return true;
}

bool ngON() {
  Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis, EFFECT_DYNAMICS_MIN_DB);  // account for F32 gate attenuation below EFFECT_DYNAMICS_MAX_DB
  noiseGateFlag = 1;
  return true;
}

bool ngOFF() {
  noiseGateFlag = 0;
  Dynamics.gate(MIN_DB, myNGattackTime, myNGreleaseTime, myNGhysterisis, 0.0f);  // account for F32 gate attenuation
  return true;
}

bool procON() {
  Dynamics.compression(myPRCthreshold, myPRCattack, myPRCrelease, myPRCratio, myPRCkneeWidth);
  procFlag = 1;
  return true;
}

bool procOFF() {
  Dynamics.compression(MAX_DB, myPRCattack, myPRCrelease, myPRCratio, myPRCkneeWidth);
  procFlag = 0;
  return true;
}

bool limON() {
  Dynamics.limit(myLIMthreshold, myLIMattack, myLIMrelease);
  limFlag = 1;
  return true;
}

bool limOFF() {
  Dynamics.limit(MAX_DB, myLIMattack, myLIMrelease);
  limFlag = 0;
  return true;
}

bool amgON() {
  Dynamics.makeupGain(0.0f);
  mupFlag = 0;
  Dynamics.autoMakeupGain(myAMGheadroom);
  amgFlag = 1;
  return true;
}

bool amgOFF() {
  Dynamics.autoMakeupGain(0.0f);
  amgFlag = 0;
  return true;
}

bool mupON() {
  Dynamics.autoMakeupGain(0.0f);
  amgFlag = 0;
  Dynamics.makeupGain(myMUPgain);
  mupFlag = 1;
  return true;
}

bool mupOFF() {
  Dynamics.makeupGain(0.0f);
  mupFlag = 0;
  return true;
}

bool inputFaderControl() {
  inputMixer.gain(1, myUSBInputFader);
  inputMixer.gain(0, 1.0f - myUSBInputFader);
  return true;
}

void MyDelay(unsigned long ms) {
  unsigned long currentMillis = millis();
  unsigned long previousMillis = millis();

  while (currentMillis - previousMillis < ms) {
    currentMillis = millis();
  }
}

/* draw long hand with given angle, scale and opacity */
void drawBKGRND() {
  // always use the method with blending, even when opacity = 1.0f because the hand has transparency
  im.blit(DSPBKGRND, 0, 0, 1.0);
}

void drawQuickMenu(int b, int c) {
  if (b != 0) {
    fps = 0;  // only reset timer if encoder isn't turning.
  }


  if (currentQuickMenuLevel == 0) {  // do this if we are on the main menu


    currentQuickMenuSelection = currentQuickMenuSelection + b;
    if (currentQuickMenuSelection > QUICK_MENU_SELECTIONS) {
      currentQuickMenuSelection = 0;
    }
    if (currentQuickMenuSelection < 0) {
      currentQuickMenuSelection = QUICK_MENU_SELECTIONS;
    }

    im.drawRect({ quickMenuBox[currentQuickMenuSelection][0], quickMenuBox[currentQuickMenuSelection][1] }, { quickMenuBox[currentQuickMenuSelection][2], quickMenuBox[currentQuickMenuSelection][3] }, RGB565_Red);


    // update data from selected box!
    if (c != 0) {
      fps = 0;  // reset timer - we are working...

      if (currentQuickMenuSelection == 0) {  // Set Mic Gain on Main menu
        micGainSet = micGainSet + c;
        if (micGainSet < 0) {
          micGainSet = 0;
        }
        if (micGainSet > 63) {
          micGainSet = 63;
        }
        audioShield.micGain(micGainSet);
      }

      if (currentQuickMenuSelection == 1) {  // Line IN Gain
        myLineInLevel = myLineInLevel + c;
        if (myLineInLevel < 0) {
          myLineInLevel = 0;
        }
        if (myLineInLevel > 15) {
          myLineInLevel = 15;
        }
        audioShield.lineInLevel(myLineInLevel);
    }

      if (currentQuickMenuSelection < 10 and currentQuickMenuSelection > 1) {
        ydBLevel[currentQuickMenuSelection - 2] = ydBLevel[currentQuickMenuSelection - 2] + c;
        if (ydBLevel[currentQuickMenuSelection - 2] > 12) {
          ydBLevel[currentQuickMenuSelection - 2] = 12;
        }
        if (ydBLevel[currentQuickMenuSelection - 2] < -12) {
          ydBLevel[currentQuickMenuSelection - 2] = -12;
        }
        if (equalizerFlag == 1) {
          EqGainSetL();
        }
      }


      if (currentQuickMenuSelection == 10) {  // Phone Volume
        myVolume = myVolume + (c * .05);     // 20 step resolution
        if (myVolume < 0) {
          myVolume = 0;
        }
        if (myVolume > 1) {
          myVolume = 1;
        }
        audioShield.volume(myVolume);
      }

    }

    if (currentQuickMenuSelection == 11) {  // lineOUtLevel
      myLineOutLevel = myLineOutLevel - (c);
      if (myLineOutLevel < 13) {
        myLineOutLevel = 13;
      }
      if (myLineOutLevel > 31) {
        myLineOutLevel = 31;
      }
      audioShield.lineOutLevel(myLineOutLevel);
    }


    if (currentQuickMenuSelection == 12) {
      if (c != 0) {
        currentMixerMenuSelection = 12;
        currentQuickMenuLevel = 1;  /// activate the mixer menu
      }
    }

    if (currentQuickMenuSelection == 13) { // Select Mic/Line in
      myInput = myInput + c;
      if (myInput > 1) {
        myInput = 0;
      }
      if (myInput < 0) {
        myInput = 1;
      }
      audioShield.inputSelect(myInput);
    }


    if (currentQuickMenuSelection == 14) {
      if (c != 0) {
        currentMixerMenuSelection = 14;
        currentQuickMenuLevel = 1;  /// activate the gate menu
      }
    }

    if (currentQuickMenuSelection == 15) {
      if (c != 0) {
        currentMixerMenuSelection = 15;
        currentQuickMenuLevel =1; /// activate the comp menu
      }
    }

    if (currentQuickMenuSelection == 16) {
      if (c != 0) {
        if (limFlag == 1) {
          limOFF();
        } else {
          limON();
        }
      }
    }

    if (currentQuickMenuSelection == 17) {
      if (c != 0) {
        if (equalizerFlag == 1) {
          eqOFF();
        } else {
          eqON();
        }
      }
    }

    if (currentQuickMenuSelection == 18) {
      if (c != 0) {
        if (AVCFlag == 1) {
          AVCoff();
        } else {
          AVCon();
        }
      }
    }

    if (currentQuickMenuSelection == 19) {
      if (c != 0) {
        if (amgFlag == 1) {
          amgOFF();
        } else {
          amgON();
        }
      }
    }

    if (currentQuickMenuSelection == 20) {
      if (c != 0) {
        if (mupFlag == 1) {
          mupOFF();
        } else {
          mupON();
        }
      }
    }
  }

  if (currentQuickMenuLevel == 1 and currentQuickMenuSelection == 12) {
    drawMixerMenu(b, c);
  }

  if (currentQuickMenuLevel == 1 and currentQuickMenuSelection == 14) {
    drawGateMenu(b, c);
  }

  if (currentQuickMenuLevel == 1 and currentQuickMenuSelection == 15) {
    drawCompMenu(b, c);
  }

}

void drawCompMenu(int b, int c) {
  int compXOffset = 40;

  if (c != 0) {  // keep the menu active while using
    fps = 0;
  }
  im.blit(Comp, (28 + compXOffset), 2, 1.0);

  currentCompMenuSelection = currentCompMenuSelection + b;
  if (currentCompMenuSelection > COMP_MENU_SELECTIONS) {
    currentCompMenuSelection = 0;
  }
  if (currentCompMenuSelection < 0) {
    currentCompMenuSelection = COMP_MENU_SELECTIONS;
  }

  im.drawRect({ compMenuBox[currentCompMenuSelection][0]+compXOffset, compMenuBox[currentCompMenuSelection][1] }, { compMenuBox[currentCompMenuSelection][2], compMenuBox[currentCompMenuSelection][3] }, RGB565_Red);

  // Draw power on light...
  if (procFlag == 1){
    im.blit(CompOn, (36 + compXOffset),71,1.0);
  }

  // Draw the display

  int startx = 172;
  int starty = 80;
  char buf[10];
  float peak;
  float peakVal;
  int peakPreM;
  float distX;
  float distY;
  
  // Threshold line
  int yThresh;  // y position of threshold (knee bend)
  yThresh = map(myPRCthreshold, -110, 0, 80, 10);
  im.drawLine({170,yThresh}, {240, yThresh},RGB565_White );
  im.drawLine({startx, starty}, {startx-yThresh+80, yThresh}, RGB565_Red);

  int aRatio; // angle between 45 and 90
  aRatio = map(myPRCratio, 1, 60, 0, 45);
  int x1 = startx-yThresh+80;
  int y1 = yThresh;
  int x2 = startx + 69;
  int y2 = y1 - ((x2-x1) * tan(aRatio * DEG2RAD));

  im.drawLine({x1,y1}, {x2,y2}, RGB565_Red);
  
  sprintf(buf, "%.1f", myPRCattack);
  im.drawText(buf, {compXOffset+50, 50}, RGB565_White, font_tgx_OpenSans_8,true );
  sprintf(buf, "%.1f", myPRCrelease);
  im.drawText(buf, {compXOffset+95, 50}, RGB565_White, font_tgx_OpenSans_8,true );
  sprintf(buf, "%.1f", myPRCkneeWidth);  
  im.drawText(buf, {compXOffset+95, 77}, RGB565_White, font_tgx_OpenSans_8,true );

  // draw pre vu along compressor curve?
  if (peakPre.available()) {
    peakVal = peakPre.read();
    peak = peakVal > 0.00065234f ? peakVal : 0.00065234f;
    compPre = mapf(peak, 0.0, 1.0, 1, 70);
  }
  if (80-compPre >  yThresh ) {
    distX = tan(45 * DEG2RAD);
    distY = distX;
      im.drawCircle({startx+(distX*compPre), starty-(distY * compPre)}, 1, RGB565_Green);
  }  else {
    distX = cos(aRatio * DEG2RAD);
    distY = sin(aRatio * DEG2RAD);
    im.drawCircle({x1+(distX*(compPre-(80-yThresh))), y1-(distY * (compPre-(80-yThresh)))}, 1, RGB565_Green);
  }  



  // Deal with the settings
  if (c !=0 ){
    if (currentCompMenuSelection == 0){       // Turn on/off comp
      if (procFlag == 1) {
        procOFF();
      } else {
        procON();
      }
    }    

    if (currentCompMenuSelection == 1) {     // Adjust Threshold
      myPRCthreshold= myPRCthreshold + c;
      if (myPRCthreshold > 0){
        myPRCthreshold = 0;
      }
      if (myPRCthreshold < -110){
        myPRCthreshold = -110;
      }
    }

    if (currentCompMenuSelection == 2) {     // Adjust Ratio
      myPRCratio= myPRCratio + c;
      if (myPRCratio > 60){
        myPRCratio = 60;
      }
      if (myPRCratio < -1){
        myPRCratio = 1;
      }
    }  

    if (currentCompMenuSelection == 3) {     // Adjust Attack
      myPRCattack= myPRCattack + (c * .1);
      if (myPRCattack > 10){
        myPRCattack = 10;
      }
      if (myPRCattack < 0){
        myPRCattack = 0;
      }
    }

    if (currentCompMenuSelection == 4) {     // Adjust Release
      myPRCrelease= myPRCrelease + ( c * .1);
      if (myPRCrelease > 10){
        myPRCrelease = 10;
      }
      if (myPRCrelease < 0){
        myPRCrelease = 0;
      }
    }

    if (currentCompMenuSelection == 5) {     // Adjust Knee
      myPRCkneeWidth= myPRCkneeWidth + c;
      if (myPRCkneeWidth > 32){
        myPRCkneeWidth = 32;
      }
      if (myPRCkneeWidth < 0){
        myPRCkneeWidth = 0;
      }
    }

  }

}

void drawGateMenu(int b, int c) {
  int gateXOffset = 40;

  if (c != 0) {  // keep the menu active while using
    fps = 0;
  }
  im.blit(Gate2, (28 + gateXOffset), 2, 1.0);

  currentGateMenuSelection = currentGateMenuSelection + b;
  if (currentGateMenuSelection > GATE_MENU_SELECTIONS) {
    currentGateMenuSelection = 0;
  }
  if (currentGateMenuSelection < 0) {
    currentGateMenuSelection = GATE_MENU_SELECTIONS;
  }

  im.drawRect({ gateMenuBox[currentGateMenuSelection][0]+gateXOffset, gateMenuBox[currentGateMenuSelection][1] }, { gateMenuBox[currentGateMenuSelection][2], gateMenuBox[currentGateMenuSelection][3] }, RGB565_Red);


  // Draw power on light...
  if (noiseGateFlag == 1){
    im.blit(GateOn, (35 + gateXOffset),75,1.0);
  }


  // Draw threshold and hysterisis lines...
  int xThreshStart = 35;
  int xThreshEnd = 204;
  int y = map(myNGthreshold, -96, 0, 69, 10);
  int h = myNGhysterisis;
  if (y+h < 69){
    im.drawLine({xThreshStart+gateXOffset,y+h}, {xThreshEnd+gateXOffset, y+h},RGB565_Yellow );  // Draw the Hysterisis line first (Thresh will cover if it's "OFF")
  }
  im.drawLine({xThreshStart+gateXOffset,y}, {xThreshEnd+gateXOffset, y},RGB565_White );

  // Draw attack line...
  int xAttackEnd = 99 + gateXOffset;
  int yAttackStart = 60;
  int yAttackEnd = 11;
  int x = map(myNGattackTime, 0, 10, 0, -60);  // maybe mapf?
  im.drawLine({xAttackEnd + x, yAttackStart},{xAttackEnd, yAttackEnd}, RGB565_Red);
  im.drawLine({35+gateXOffset, yAttackStart}, {xAttackEnd + x, yAttackStart}, RGB565_Red);

  // Draw release line...
  int xReleaseEnd = 133+gateXOffset;
  int yReleaseStart = 60;
  int yReleaseEnd = 11;
  x = map(myNGreleaseTime, 0, 10, 0, 60);  // maybe mapf?
  im.drawLine({xReleaseEnd + x, yReleaseStart},{xReleaseEnd, yReleaseEnd}, RGB565_Red);
  im.drawLine({xAttackEnd, yAttackEnd}, {xReleaseEnd, yReleaseEnd}, RGB565_Red);
  im.drawLine({xReleaseEnd + x, yReleaseStart}, {xThreshEnd+gateXOffset, yReleaseStart}, RGB565_Red);


  // Deal with the settings
  if (c !=0 ){
    if (currentGateMenuSelection == 0){       // Turn on/off gate
      if (noiseGateFlag == 1) {
        ngOFF();
      } else {
        ngON();
      }
    }    

    if (currentGateMenuSelection == 1){      // Adjust Threshold -96 to 0 dBFS
      myNGthreshold = myNGthreshold + c;
      if (myNGthreshold > EFFECT_DYNAMICS_MAX_DB){
        myNGthreshold = EFFECT_DYNAMICS_MAX_DB;
      }
      if (myNGthreshold < EFFECT_DYNAMICS_MIN_DB) {
        myNGthreshold = EFFECT_DYNAMICS_MIN_DB;
      }
      // Update the settings
      Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis, EFFECT_DYNAMICS_MIN_DB);  // account for F32 gate attenuation
    }

    if (currentGateMenuSelection == 2){     // Adjust the Attack 1 to 10 step .001
      myNGattackTime = myNGattackTime + (c * .1);
      if (myNGattackTime > 10){
        myNGattackTime = 10;
      }
      if (myNGattackTime < 0){
        myNGattackTime = 0;
      }
      // Update the settings
      Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis, EFFECT_DYNAMICS_MIN_DB);  // account for F32 gate attenuation
    }

    if (currentGateMenuSelection == 3){     // Adjust the Release 1 to 10 step .1
      myNGreleaseTime = myNGreleaseTime + (c * .1);
      if (myNGreleaseTime > 10){
        myNGreleaseTime = 10;
      }
      if (myNGreleaseTime < 0){
        myNGreleaseTime = 0;
      }
      // Update the settings
      Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis, EFFECT_DYNAMICS_MIN_DB);  // account for F32 gate attenuation
    }


    if (currentGateMenuSelection == 4){     // Adjust the Hysterisis 1 to 12 step .1
      myNGhysterisis = myNGhysterisis + (c * .1);
      if (myNGhysterisis > 12){
        myNGhysterisis = 12;
      }
      if (myNGhysterisis < 0){
        myNGhysterisis = 0;
      }
       // Update the settings
      Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis, EFFECT_DYNAMICS_MIN_DB);  // account for F32 gate attenuation
    }
  }
}


void drawMixerMenu(int b, int c) {
  int mixerXOffset = 42;

  if (c != 0) {  // keep the menu active while using
    fps = 0;
  }

  im.blit(Mixer, 28+mixerXOffset, 4, 1.0);

  currentMixerMenuSelection = currentMixerMenuSelection + b;
  if (currentMixerMenuSelection > MIXER_MENU_SELECTIONS) {
    currentMixerMenuSelection = 0;
  }
  if (currentMixerMenuSelection < 0) {
    currentMixerMenuSelection = MIXER_MENU_SELECTIONS;
  }

  im.drawRect({ mixerMenuBox[currentMixerMenuSelection][0]+mixerXOffset, mixerMenuBox[currentMixerMenuSelection][1] }, { mixerMenuBox[currentMixerMenuSelection][2], mixerMenuBox[currentMixerMenuSelection][3] }, RGB565_Red);


  // deal with settings....

  if (c != 0) {
    if (currentMixerMenuSelection == 0) {  // Mic IIN GainN Gain
      micGainSet = micGainSet + c;
      if (micGainSet < 0) {
        micGainSet = 0;
      }
      if (micGainSet > 63) {
        micGainSet = 63;
      }
      audioShield.micGain(micGainSet);
    }


    if (currentMixerMenuSelection == 1) {  // Line IN Gain
      myLineInLevel = myLineInLevel + c;
      if (myLineInLevel < 0) {
        myLineInLevel = 0;
      }
      if (myLineInLevel > 15) {
        myLineInLevel = 15;
      }
      audioShield.lineInLevel(myLineInLevel);
    }


    if (currentMixerMenuSelection == 2) {             // USB/INPUT fader
      myUSBInputFader = myUSBInputFader + (c * .05);  // .05 (20 step resolution)
      if (myUSBInputFader < 0) {
        myUSBInputFader = 0;
      }
      if (myUSBInputFader > 1) {
        myUSBInputFader = 1;
      }
      inputFaderControl();
    }


    if (currentMixerMenuSelection == 3) {     // DAC Volume
      myDACVolume = myDACVolume + (c * .05);  // 20 step resolution
      if (myDACVolume < 0) {
        myDACVolume = 0;
      }
      if (myDACVolume > 1) {
        myDACVolume = 1;
      }
      audioShield.dacVolume(myDACVolume);
    }


    if (currentMixerMenuSelection == 4) {  // Phone Volume
      myVolume = myVolume + (c * .05);     // 20 step resolution
      if (myVolume < 0) {
        myVolume = 0;
      }
      if (myVolume > 1) {
        myVolume = 1;
      }
      audioShield.volume(myVolume);
    }

    if (currentMixerMenuSelection == 5) {  // lineOUtLevel
      myLineOutLevel = myLineOutLevel - (c);
      if (myLineOutLevel < 13) {
        myLineOutLevel = 13;
      }
      if (myLineOutLevel > 31) {
        myLineOutLevel = 31;
      }
      audioShield.lineOutLevel(myLineOutLevel);
    }
  }


  // Draw sliders
  int y;
  y = map(micGainSet, 0, 63, 0, 45);
  im.blit(EQSlider, 38+mixerXOffset, 65 - y, 1.0);  // Mic Gain

  y = map(myLineInLevel, 0, 15, 0, 45);
  im.blit(EQSlider, 63+mixerXOffset, 65 - y, 1.0);  // Line In Gain

  y = map(myUSBInputFader, 0, 1, 0, 45);
  im.blit(EQSlider, 99+mixerXOffset, 65 - y, 1.0);  // USB/Level Fader

  y = map(myDACVolume, 0, 1, 0, 45);
  im.blit(EQSlider, 137+mixerXOffset, 65 - y, 1.0);  // DAC Volume

  y = map(myVolume, 0, 1, 0, 45);
  im.blit(EQSlider, 161+mixerXOffset, 65 - y, 1.0);  // Headphone Volume

  y = map(myLineOutLevel, 31, 13, 0, 45);
  im.blit(EQSlider, 185+mixerXOffset, 65 - y, 1.0);  // myLineOutLevel Volume
}




// Floating point map function declatration
double mapf(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void setup(void) {
  // deleteFile();  - this fixed my line in audio issue
  Serial.begin(115200);
  delay(500);
  Serial.println("Setup starting...");

  // display
  while (!display.begin(SPI_SPEED))
    ;

  display.setScroll(0);
  display.setRotation(1);
  display.setFramebuffer(ib);  // set 1 internal framebuffer -> activate float buffering
  //display.setDiffBuffers(&diff1, &diff2);  // set the 2 diff buffers => activate differential updates
  display.setDiffGap(4);       // use a small gap for the diff buffers
  display.setRefreshRate(60);  // around 120hz for the display refresh rate
  display.setVSyncSpacing(1);  // set framerate = refreshrate/2 (and enable vsync at the same time)

  // make sure backlight is on
  if (PIN_BACKLIGHT != 255) {
    pinMode(PIN_BACKLIGHT, OUTPUT);
    digitalWrite(PIN_BACKLIGHT, HIGH);
  }
  Serial.println("Display running..");

  Timer1.initialize(500);
  Timer1.attachInterrupt(timerIsr);
  Serial.println("Knob encoder interrupt routines started.");

  AudioMemory(20);
  AudioMemory_F32(20);
  audioShield.enable();
  audioShield.muteHeadphone();
  audioShield.muteLineout();
  audioShield.dacVolumeRamp();  // smmothly move between volume levels to avoid pops and clocks
  audioShield.audioProcessorDisable();
  audioShield.inputSelect(myInput);
  audioShield.lineInLevel(0, 0);           // Sane start point before config restore
  audioShield.volume(0.5);                 // ditto
  audioShield.adcHighPassFilterDisable();  // no need for DCblocking filter. physical filter exists
  Serial.println("Audio subsystem up");

  fftValues.windowFunction(AudioWindowBlackmanHarris1024);
  fftValues.setNAverage(1);
  fftValues.setOutputType(FFT_DBFS);  // graphs real nice on our display

  uint16_t eq = equalize.equalizerNew(8, &fBand[0], &dbBand[0], 30, &equalizeCoeffs[0], 60.0);
  if (eq == ERR_EQ_BANDS)
    Serial.println("Equalizer failed: Invalid number of frequency bands.");
  else if (eq == ERR_EQ_SIDELOBES)
    Serial.println("Equalizer failed: Invalid sidelobe specification.");
  else if (eq == ERR_EQ_NFIR)
    Serial.println("Equalizer failed: Invalid number of FIR coefficients.");
  else
    Serial.println("Equalizer initialized successfully.");

  readFromFile();  //  Check for and restore last save state if present
  Serial.println("resotred last config state");
  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
  Serial.println("Outputs unmuted");

  Serial.print("Audio Memory in use:");
  Serial.println(AudioMemoryUsage());
  Serial.print("CPU: Max Percent Usage: ");
  Serial.println(AudioProcessorUsageMax());
  Serial.print("   Max Float 32 Memory: ");
  Serial.println(AudioMemoryUsageMax_F32());
}

void loop() {

  b = clickEncoder.getValue();
  c = clickEncoder2.getValue();

  drawBKGRND();
  drawFFT();
  drawVUmeters();
  drawEQSliders();
  //drawCompressor();
  drawInOutSliders();
  drawButtons();
  display.overlayFPS(fb);

  if (b != 0 or fps < 5000) {
    drawQuickMenu(b, c);
  } else {
    currentQuickMenuLevel = 0;
  }

  display.update(fb);
  yield();

  if (spectrumFlag == -1) {  // turn this off
    b = clickEncoder.getButton();
    if (b == ClickEncoder::Clicked) {
      spectrumFlag = 0;
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

  audioShield.audioProcessorDisable();

  EqGainSetL();  // Setup Equalizer coefficients before turning on.. avoids pop

  if (equalizerFlag == 1)
    eqON();
  else
    eqOFF();

  /* Dynamics - Order of events:
      1. gate -> timeToAlpha -> timeToAlpha
      2. compression -> computeMakeupGain -> timeToAlpha -> timeToAlpha
      3. limit -> computeMakeupGain -> timeToAlpha -> timeToAlpha
      4. makeupgain
      5. autoMakeupGain -> computeMakeupGain
    */

  if (noiseGateFlag == 1)
    ngON();
  else
    ngOFF();

  if (procFlag == 1)
    procON();
  else
    procOFF();

  if (limFlag == 1)
    limON();
  else
    limOFF();

  if (mupFlag == 1)
    mupON();
  else
    mupOFF();

  if (amgFlag == 1)
    amgON();
  else
    amgOFF();

  audioShield.unmuteHeadphone();
  audioShield.unmuteLineout();
}

void SetLevels() {
  audioShield.volume(myVolume);
  audioShield.micGain(micGainSet);
  audioShield.lineInLevel(myLineInLevel);
  audioShield.lineOutLevel(myLineOutLevel);
  if (amgFlag == 1)
    Dynamics.autoMakeupGain(myAMGheadroom);
  if (mupFlag == 1)
    Dynamics.makeupGain(myMUPgain);
}

void EqGainSetL() {
  for (int freqBand = 0; freqBand < 8; freqBand++) {
    dbBand[freqBand] = ydBLevel[freqBand];
  }
  // punch the filter again...
  equalize.equalizerNew(8, &fBand[0], &dbBand[0], 30, &equalizeCoeffs[0], 60.0);
}

void drawButtons() {
  if (myInput == AUDIO_INPUT_MIC) {
    im.blit(Mic_On, 16, 96, 1.0);
  }

  if (noiseGateFlag == 1) {
    im.blit(Gate_On, 53, 96, 1.0);
  }

  if (procFlag == 1) {
    im.blit(Comp_On, 89, 96, 1.0);
  }

  if (limFlag == 1) {
    im.blit(Limit_On, 126, 96, 1.0);
  }

  if (equalizerFlag == 1) {
    im.blit(EQ_On, 163, 96, 1.0);
  }

  if (AVCFlag == 1) {
    im.blit(AVolOn, 200, 96, 1.0);
  }

  if (amgFlag == 1) {
    im.blit(AMuOn, 236, 96, 1.0);
  }

  if (mupFlag == 1) {
    im.blit(MkUpOn, 274, 96, 1.0);
  }
}

void drawCompressor() {
  // display is 64 x 75

  int xThresh = 216;  // width = 75
  int yThresh = 73;

  // draw threshold line...
  int x = map(myPRCthreshold, -110, 0, 0, 72);

  im.drawLine({ xThresh + x, yThresh }, { xThresh + x, yThresh - 64 }, RGB565_Blue);

  // draw reference line
  int x1 = xThresh;
  int y1 = yThresh;
  int x2 = x1 + x;
  int y2 = y1 - x * tan(41.634 * DEG2RAD);

  im.drawLine({ x1, y1 }, { x2, y2 }, RGB565_White);

  // draw the ratio
  int ratio = map(myPRCratio, 1.0, 60.0, 41, 0);
  ratio = 20;  // hard coding ratio display for tsting

  x1 = x2;
  y1 = y2;
  x2 = xThresh + 72;
  y2 = y1 - ((x2 - x1) * tan(ratio * DEG2RAD));

  im.drawLine({ x1, y1 }, { x2, y2 }, RGB565_White);
}

void drawVUmeters() {
  const int posXin = 55;
  const int posYin = 73;
  const int posXout = 255;
  const int posYout = 73;
  //const int minHeight = 1;
  const int maxHeight = 33;
  //float peakFloat = 1.2;
  float peak, peakVal;
  float peakPreM = 0.0;
  float peakPostM = 0.0;

  if (peakPre.available()) {
    peakVal = peakPre.read();
    //peak = peakVal > 0.00065234f ? 20.0f*log10f(peakVal) : -65.0;  // Convert to dB and clip lower bounds just above noise floor
    peak = peakVal > 0.00065234f ? peakVal : 0.00065234f;
    peakPreM = mapf(peak, 0.0, 1.0, 1.0, 33.0);
    for (int i = 0; i < maxHeight; i++) {
      if (i < int(peakPreM)) {
        im.blit(LITGREENVU, posXin, posYin - (i * 2), 1.0);
      }
    }
  }

  if (peakPost.available()) {
    peakVal = peakPost.read();
    //peak = peakVal > 0.00065234f ? 20.0f*log10f(peakVal) : -65.0; // Convert to dB and clip lower bounds just above noise floor
    peak = peakVal > 0.00065234f ? peakVal : 0.00065234f;
    peakPostM = mapf(peak, 0, 1.0, 1.0, 33.0);
    for (int i = 0; i < maxHeight; i++) {
      if (i < int(peakPostM)) {
        im.blit(LITGREENVU, posXout, posYout - (i * 2), 1.0);
      }
    }
  }
}

void drawEQSliders() {

  const int xSlider = 77;
  const int ySlider = 70;
  int y;

  for (int i = 0; i < 8; i++) {
    y = map(ydBLevel[i], -12, 12, 0, 64);
    im.blit(EQSlider, xSlider + (i * 22), ySlider - y, 1.0);
  }
}

void drawInOutSliders() {

  // Draw IN/OUT sliders
  int y;
  y = map(micGainSet, 0, 63, 0, 45);
  im.blit(EQSlider, 17-7, 65 - y, 1.0);  // Mic Gain

  y = map(myLineInLevel, 0, 15, 0, 45);
  im.blit(EQSlider, 37-7, 65 - y, 1.0);  // Line In Gain

  y = map(myVolume, 0, 1, 0, 45);
  im.blit(EQSlider, 283-7, 65 - y, 1.0);  // Headphone Volume

  y = map(myLineOutLevel, 31, 13, 0, 45);
  im.blit(EQSlider, 303-7, 65 - y, 1.0);  // myLineOutLevel Volume

}

void drawFFT() {
  const int nBars = sizeof(fftOctTab) / 4;  // Sweet uint16_t batman...
  const int barWidth = 15;
  const int posX = 29;
  const int posY = 229;
  //const int minHeight = 1;
  const int maxHeight = 36;
  int mVal = 0;
  int delta = 0;
  float n;
  // int16_t val;
  float peakFloat = 1.2;

  ///////////////////////////////////////////////////////////////////////
  //  FFT display
  //////////////////////////////////////////////////////////////////////

  if (fftValues.available()) {

    for (int i = 0; i < nBars - 1; i++)  // was getting another FFT bar off the RT of the screen...
    {
      n = fftValues.read(fftOctTab[i * 2], fftOctTab[i * 2 + 1]);
      delta = fftOctTab[i * 2 + 1] - fftOctTab[i * 2];
      if (delta > 0) {
        n = n / delta;  // divide by number of bins in the sample to make average to graph
      } else {
        n = n * 2;  // edge case of first bin..
      }
      mVal = map(constrain(int(n), -80, 0), -80, 0, 0, 36); // check into the constrain and if the int() is actaully ok...
      int x = posX + i * barWidth;

      if (mVal >= barPeak[i]) {
        barPeak[i] = mVal;
      } else {
        barPeak[i] = barPeak[i] - peakFloat;
      }
      for (int barNum = 0; barNum < maxHeight; barNum++) {
        if (barNum < mVal) {
          im.blit(LITGREEN, x, posY - (barNum * 3), 1.0);
        }

        if (barNum == round(barPeak[i])) {
          im.blit(LITWHITE, x, posY - (barNum * 3), 1.0);
        }
      }
    }
  }
}

void readFromFile() {
  boolean they_match = true;

  EEPROM.get(0, tempMagic);
  for (int i = 0; i < 4; i++) {
    if (tempMagic[i] != EEPROM_MAGIC[i]) {
      they_match = false;
      break;
    }
  }
  if (they_match) {
    int address = sizeof(EEPROM_MAGIC);
    EEPROM.get(address, mySaveSet);
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
  }
  SetAudioShield();
}

void writeToFile() {
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
  mySaveSet.noiseGateFlag = noiseGateFlag;
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

  EEPROM.put(0, EEPROM_MAGIC);
  int address = sizeof(EEPROM_MAGIC);
  EEPROM.put(address, mySaveSet);
}

void deleteFile() {
  EEPROM.put(0, EEPROM_INIT);
}

void resetDefault(void) {
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
}