#include "OpenAudio_ArduinoLibrary.h" // https://github.com/chipaudette/OpenAudio_ArduinoLibrary
#include "AudioStream_F32.h"
#include "USB_Audio_F32.h"       // But why not?!?!?
#include "effect_dynamics_F32.h" // Add this: https://github.com/MarkzP/AudioEffectDynamics_F32

// Not sure why I had to do this, they are included in the line previous....
#ifndef MIN_DB
#define MIN_DB -110.0f
#endif
#ifndef MAX_DB
#define MAX_DB 0.0f
#endif

#include <Audio.h>
#include <EEPROM.h> // store the config here for power persistance
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <ILI9341_T4.h> // the screen driver library : https://github.com/vindar/ILI9341_T4
#include <tgx.h>        // the tgx library
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

#define enc2A 17 // DT
#define enc2B 16 // CLK
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

void timerIsr()
{
    clickEncoder.service();
    clickEncoder2.service();
}

// Let's get programmatic about our settings and not take the defaults, eh?
const float sample_rate_Hz = 44100.0f; // overboard for our appliation, but why not?
const int audio_block_samples = 128;  // max numb of blocks, and the default
AudioSettings_F32 audio_settings(sample_rate_Hz, audio_block_samples);

AudioControlSGTL5000 audioShield; 

AudioInputI2S_F32 audioInput(audio_settings);      
AudioInputUSB_F32 audioInUSB1(audio_settings);   
AudioOutputI2S_F32 audioOutput(audio_settings);  

AudioMixer4_F32 inputMixer(audio_settings);       
AudioAnalyzePeak_F32 peakPre;      
AudioSwitch4_OA_F32 eqSwitch;     
AudioFilterEqualizer_F32 equalize(audio_settings); //https://forum.pjrc.com/threads/60928-Audio-Equalizer-using-FIR
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
int currentQuickMenuSelection = 0;
int quickMenuBox[21][4] = {
    {4,  6, 23, 78},   // IN Gain
    {31, 6, 24, 78},   // EQ 1
    {53, 6, 24, 78},   // EQ 2
    {75, 6, 24, 78},   // EQ 3
    {98, 6, 24, 78},   // EQ 4
    {120, 6, 24, 78},  // EQ 5
    {142, 6, 24, 78},  // EQ 6
    {164, 6, 24, 78},  // EQ 7
    {187, 6, 24, 78},  // EQ 8
    {212, 6, 81, 78},  // Compressor
    {295, 6, 23, 79},  // Out Gain
    {14, 94, 36, 22},  // Line
    {50, 94, 36, 22},  // GAte
    {87, 94, 36, 22},  // Comp
    {124, 94, 36, 22}, // LImit
    {161, 94, 36, 22}, // EQ
    {198, 94, 37, 22}, // A-Vol
    {235, 94, 37, 22}, // A-Mu
    {272, 94, 37, 22}  // MkUp
};

const int QUICK_MENU_SELECTIONS = 18;

elapsedMillis fps;

// EEPROM Magic - if present then we have a config to load
int EEPROM_MAGIC[4] = {0xFE, 0xED, 0xC0, 0xDE}; // https://en.wikipedia.org/wiki/Hexspeak for the LOLs
int EEPROM_INIT[4] = {0x00, 0x00, 0x00, 0x00};  // invalid config
int tempMagic[4] = {0x00, 0x00, 0x00, 0x00};

struct ConfigSaveSet
{
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
float32_t fBand[] = {150.0, 240.0, 370.0, 590.0, 900.0, 1300.0, 2000.0, 3300.0};
float32_t dbBand[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
float32_t equalizeCoeffs[200];

float bandGain[] = {1, 1, 1, 1, 1, 1, 1, 1};
float ydBLevel[] = {0, 0, 0, 0, 0, 0, 0, 0};

int equalizerFlag = EQUALIZERFLAG;
int noiseGateFlag = NOISEGATEFLAG;
int myLineInLevel = MYLINEINLEVEL;   // range is 0 to 15
int myLineOutLevel = MYLINEOUTLEVEL; // range is 13 to 31
float myVolume = MYVOLUME;           // 0.8 is max for undistorted headphone output
float micGainSet = MICGAINSET;       // 35 for the Chinese MH-1B8 mic
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
  398, 511 };
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
DMAMEM uint16_t ib[LX * LY]; // used for internal buffering
DMAMEM uint16_t fb[LX * LY]; // paint in this buffer

// image that encapsulates fb.
Image<RGB565> im(fb, LX, LY);

//
// DEFAULT WIRING USING SPI 0 ON TEENSY 4/4.1
//
#define PIN_SCK 13  // mandatory
#define PIN_MISO 12 // mandatory
#define PIN_MOSI 11 // mandatory
#define PIN_DC 10   // 10 mandatory, can be any pin but using pin 10 (or 36 or 37 on T4.1) provides greater performance

#define PIN_CS 9         //  9 optional (but recommended), can be any pin.
#define PIN_RESET 22     // 22 optional (but recommended), can be any pin.
#define PIN_BACKLIGHT 1  //  1 optional, set this only if the screen LED pin is connected directly to the Teensy.
#define PIN_TOUCH_IRQ 14 // 14 optional. set this only if the touchscreen is connected on the same SPI bus
#define PIN_TOUCH_CS 6   //  6 optional. set this only if the touchscreen is connected on the same spi bus

// Setting the screen driver for the Spectrum Display
// the screen driver object
ILI9341_T4::ILI9341Driver display(PIN_CS, PIN_DC, PIN_SCK, PIN_MOSI, PIN_MISO, PIN_RESET, PIN_TOUCH_CS, PIN_TOUCH_IRQ);

// Equalizer on
bool eqON()
{
    equalizerFlag = 1;
    eqSwitch.setChannel(0);
    return true;
}

// Equalizer off
bool eqOFF()
{
    equalizerFlag = 0;
    eqSwitch.setChannel(1);
    return true;
}

bool AVCon()
{
  // Disabled for now
    AVCFlag = 1;
    return true;
}

// AVC off
bool AVCoff()
{
    // Disabled for now
    AVCFlag = 0;
    return true;
}

bool ngON()
{
    Dynamics.gate(myNGthreshold, myNGattackTime, myNGreleaseTime, myNGhysterisis, 0.0f); // account for F32 gate attenuation
    noiseGateFlag = 1;
    return true;
}

bool ngOFF()
{
    noiseGateFlag = 0;
    Dynamics.gate(MIN_DB, myNGattackTime, myNGreleaseTime, myNGhysterisis, 0.0f); // account for F32 gate attenuation
    return true;
}

bool procON()
{
    Dynamics.compression(myPRCthreshold, myPRCattack, myPRCrelease, myPRCratio, myPRCkneeWidth);
    procFlag = 1;
    return true;
}

bool procOFF()
{
    Dynamics.compression(MAX_DB, myPRCattack, myPRCrelease, myPRCratio, myPRCkneeWidth);
    procFlag = 0;
    return true;
}

bool limON()
{
    Dynamics.limit(myLIMthreshold, myLIMattack, myLIMrelease);
    limFlag = 1;
    return true;
}

bool limOFF()
{
    Dynamics.limit(MAX_DB, myLIMattack, myLIMrelease);
    limFlag = 0;
    return true;
}

bool amgON()
{
    Dynamics.makeupGain(0.0f);
    mupFlag = 0;
    Dynamics.autoMakeupGain(myAMGheadroom);
    amgFlag = 1;
    return true;
}

bool amgOFF()
{
    Dynamics.autoMakeupGain(0.0f);
    amgFlag = 0;
    return true;
}

bool mupON()
{
    Dynamics.autoMakeupGain(0.0f);
    amgFlag = 0;
    Dynamics.makeupGain(myMUPgain);
    mupFlag = 1;
    return true;
}

bool mupOFF()
{
    Dynamics.makeupGain(0.0f);
    mupFlag = 0;
    return true;
}

void MyDelay(unsigned long ms)
{
    unsigned long currentMillis = millis();
    unsigned long previousMillis = millis();

    while (currentMillis - previousMillis < ms)
    {
        currentMillis = millis();
    }
}

/* draw long hand with given angle, scale and opacity */
void drawBKGRND()
{
    // always use the method with blending, even when opacity = 1.0f because the hand has transparency
    im.blit(DSPBKGRND, 0, 0, 1.0);
}

void drawQuickMenu(int b, int c)
{
    if (b != 0)
    {
        fps = 0; // only reset timer if encoder isn't turning.
    }
    currentQuickMenuSelection = currentQuickMenuSelection + b;
    if (currentQuickMenuSelection > QUICK_MENU_SELECTIONS)
    {
        currentQuickMenuSelection = 0;
    }
    if (currentQuickMenuSelection < 0)
    {
        currentQuickMenuSelection = QUICK_MENU_SELECTIONS;
    }

    im.drawRect({quickMenuBox[currentQuickMenuSelection][0], quickMenuBox[currentQuickMenuSelection][1]}, {quickMenuBox[currentQuickMenuSelection][2], quickMenuBox[currentQuickMenuSelection][3]}, RGB565_Red);

    // draw the IN gain graphics here...
    if (currentQuickMenuSelection == 0){
      int y;
      y = map(myLineInLevel, 0, 15, 60, 2);
      im.fillRect({5, (7 + y)}, {22, 5}, RGB565_Gray);
      im.fillRect({6, (8 + y)}, {20, 3}, RGB565_Black);
      im.fillRect({7, (9 + y)}, {18, 1}, RGB565_Cyan);
    }

    // draw the OUT gain graphics here...
    if (currentQuickMenuSelection == 10){
      int y;
      y = map(myLineOutLevel, 13, 31, 60, 2);
      im.fillRect({296, (7 + y)}, {22, 5}, RGB565_Gray);
      im.fillRect({297, (8 + y)}, {20, 3}, RGB565_Black);
      im.fillRect({298, (9 + y)}, {18, 1}, RGB565_Cyan);
    }

    // update data from selected box!
    if (c != 0)
    {
        fps = 0; // reset timer - we are working...

      if (currentQuickMenuSelection == 0) {
        myLineInLevel = myLineInLevel + c;
        if (myLineInLevel > 15){
          myLineInLevel = 15;
        }
        if (myLineInLevel < 0){
          myLineInLevel = 0;
        }
        inputMixer.gain(0,mapf(myLineInLevel,0,15,0.0f, 1.0f));
        inputMixer.gain(1,mapf(myLineInLevel,0,15,0.0f, 1.0f));
      }
        

      if (currentQuickMenuSelection < 10 and currentQuickMenuSelection > 1){
            ydBLevel[currentQuickMenuSelection-1] = ydBLevel[currentQuickMenuSelection-1] + c;
            if (ydBLevel[currentQuickMenuSelection-1] > 12)
            {
                ydBLevel[currentQuickMenuSelection-1] = 12;
            }
            if (ydBLevel[currentQuickMenuSelection-1] < -12)
            {
                ydBLevel[currentQuickMenuSelection-1] = -12;
            }
            if(equalizerFlag == 1)  {
              EqGainSetL();
            }
        }


        if (currentQuickMenuSelection == 9)
        {
            myPRCratio = 30.0;                   // for testing
            myPRCthreshold = myPRCthreshold + c; // , -110, 0,
            if (myPRCthreshold > 0)
            {
                myPRCthreshold = 0;
            }
            if (myPRCthreshold < -110)
            {
                myPRCthreshold = -110;
            }
            Serial.print(myPRCthreshold);
        }

        if (currentQuickMenuSelection == 10) {
          myLineOutLevel = myLineOutLevel + c;
          if (myLineOutLevel > 31){
            myLineOutLevel = 31;
          }
          if (myLineOutLevel < 13){
            myLineOutLevel = 13;
          }
        audioShield.volume(mapf(myLineOutLevel,13,31,0.0f, 0.8f));  //Headphones and constrained to 0.8f max recommended before distortion.
        audioShield.lineOutLevel(myLineOutLevel); //Line Out
        }


        if (currentQuickMenuSelection == 11)
        {
            myInput = myInput + c;
            if (myInput > 1)
            {
                myInput = 0;
            }
            if (myInput < 0)
            {
                myInput = 1;
            }
        }

        if (currentQuickMenuSelection == 12)
        {
            if (c != 0)
            {
                if (noiseGateFlag == 1)
                {
                    ngOFF();
                }
                else
                {
                    ngON();
                }
            }
        }

        if (currentQuickMenuSelection == 13)
        {
            if (c != 0)
            {
                if (procFlag == 1)
                {
                    procOFF();
                }
                else
                {
                    procON();
                }
            }
        }

        if (currentQuickMenuSelection == 14)
        {
            if (c != 0)
            {
                if (limFlag == 1)
                {
                    limOFF();
                }
                else
                {
                    limON();
                }
            }
        }

        if (currentQuickMenuSelection == 15)
        {
            if (c != 0)
            {
                if (equalizerFlag == 1)
                {
                    eqOFF();
                }
                else
                {
                    eqON();
                }
            }
        }

        if (currentQuickMenuSelection == 16)
        {
            if (c != 0)
            {
                if (AVCFlag == 1)
                {
                    AVCoff();
                }
                else
                {
                    AVCon();
                }
            }
        }

        if (currentQuickMenuSelection == 17)
        {
            if (c != 0)
            {
                if (amgFlag == 1)
                {
                    amgOFF();
                }
                else
                {
                    amgON();
                }
            }
        }

        if (currentQuickMenuSelection == 18)
        {
            if (c != 0)
            {
                if (mupFlag == 1)
                {
                    mupOFF();
                }
                else
                {
                    mupON();
                }
            }
        }
    }
}

// Floating point map function declatration
double mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void setup(void)
{

    Serial.begin(115200);
    delay(500);
    Serial.println("Setup starting...");

    // display
    while (!display.begin(SPI_SPEED));

    display.setScroll(0);
    display.setRotation(1);
    display.setFramebuffer(ib); // set 1 internal framebuffer -> activate float buffering
    //display.setDiffBuffers(&diff1, &diff2);  // set the 2 diff buffers => activate differential updates
    display.setDiffGap(4);       // use a small gap for the diff buffers
    display.setRefreshRate(60); // around 120hz for the display refresh rate
    display.setVSyncSpacing(1);  // set framerate = refreshrate/2 (and enable vsync at the same time)

    // make sure backlight is on
    if (PIN_BACKLIGHT != 255)
    {
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
    audioShield.dacVolumeRamp(); // smmothly move between volume levels to avoid pops and clocks
    audioShield.audioProcessorDisable();
    audioShield.inputSelect(myInput);
    audioShield.lineInLevel(0, 0);  // Sane start point before config restore
    audioShield.volume(0.5); // ditto
    audioShield.adcHighPassFilterDisable(); // no need for DCblocking filter. physical filter exists
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

    readFromFile(); //  Check for and restore last save state if present
    Serial.println("resotred last config state");
    audioShield.unmuteHeadphone();
    audioShield.unmuteLineout();
    Serial.println("Outputs unmuted");

    Serial.print("Audio Memory in use:"); Serial.println(AudioMemoryUsage());
    Serial.print("CPU: Max Percent Usage: ");
    Serial.println(AudioProcessorUsageMax());
    Serial.print("   Max Float 32 Memory: ");
    Serial.println(AudioMemoryUsageMax_F32());
}

void loop()
{

    b = clickEncoder.getValue();
    c = clickEncoder2.getValue();

    drawBKGRND();
    drawFFT();
    drawVUmeters();
    drawEQSliders();
    drawCompressor();
    drawButtons();
    display.overlayFPS(fb);

    if (b != 0 or fps < 5000)
    {
        drawQuickMenu(b, c);
    }

    display.update(fb);
    yield();

    if (spectrumFlag == -1)
    { // turn this off
        ////Serial.println("Loop2");

        b = clickEncoder.getButton();
        if (b == ClickEncoder::Clicked)
        {
            // Serial.println("Clicked");
            spectrumFlag = 0;
            //      display.fillScreen(ILI9341_BLACK);
            // display.display();
            //      nav.refresh();
            // Serial.println();
            // Serial.println("Exiting Spectrum");
        }
    }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void SetAudioShield()
{
    audioShield.muteHeadphone();
    audioShield.muteLineout();

    audioShield.inputSelect(myInput);
    audioShield.micGain(micGainSet);

    audioShield.lineInLevel(myLineInLevel);
    audioShield.lineOutLevel(myLineOutLevel);

    audioShield.volume(myVolume);

    audioShield.audioProcessorDisable();

    EqGainSetL();       // Setup Equalizer coefficients before turning on.. avoids pop

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

void SetLevels()
{
    audioShield.volume(myVolume);
    audioShield.micGain(micGainSet);
    audioShield.lineInLevel(myLineInLevel);
    audioShield.lineOutLevel(myLineOutLevel);
    if (amgFlag == 1)
        Dynamics.autoMakeupGain(myAMGheadroom);
    if (mupFlag == 1)
        Dynamics.makeupGain(myMUPgain);
}

void EqGainSetL()
{
    for (int freqBand = 0; freqBand < 8; freqBand++)
    {
        dbBand[freqBand] = ydBLevel[freqBand];
    }
    // punch the filter again...
    equalize.equalizerNew(8, &fBand[0], &dbBand[0], 30, &equalizeCoeffs[0], 60.0);
}

void drawButtons()
{
    if (myInput == AUDIO_INPUT_MIC)
    {
        im.blit(Mic_On, 16, 96, 1.0);
    }

    if (noiseGateFlag == 1)
    {
        im.blit(Gate_On, 53, 96, 1.0);
    }

    if (procFlag == 1)
    {
        im.blit(Comp_On, 89, 96, 1.0);
    }

    if (limFlag == 1)
    {
        im.blit(Limit_On, 126, 96, 1.0);
    }

    if (equalizerFlag == 1)
    {
        im.blit(EQ_On, 163, 96, 1.0);
    }

    if (AVCFlag == 1)
    {
        im.blit(AVolOn, 200, 96, 1.0);
    }

    if (amgFlag == 1)
    {
        im.blit(AMuOn, 236, 96, 1.0);
    }

    if (mupFlag == 1)
    {
        im.blit(MkUpOn, 274, 96, 1.0);
    }
}

void drawCompressor()
{
    // display is 64 x 75

    int xThresh = 216; // width = 75
    int yThresh = 73;

    // draw threshold line...
    int x = map(myPRCthreshold, -110, 0, 0, 72);

    im.drawLine({xThresh + x, yThresh}, {xThresh + x, yThresh - 64}, RGB565_Blue);

    // draw reference line
    int x1 = xThresh;
    int y1 = yThresh;
    int x2 = x1 + x;
    int y2 = y1 - x * tan(41.634 * DEG2RAD);

    im.drawLine({x1, y1}, {x2, y2}, RGB565_White);

    // draw the ratio
    int ratio = map(myPRCratio, 1.0, 60.0, 41, 0);
    ratio = 20; // hard coding ratio display for tsting

    x1 = x2;
    y1 = y2;
    x2 = xThresh + 72;
    y2 = y1 - ((x2 - x1) * tan(ratio * DEG2RAD));

    im.drawLine({x1, y1}, {x2, y2}, RGB565_White);
}

void drawVUmeters()
{
    const int posXin = 9;
    const int posYin = 73;
    const int posXout = 300;
    const int posYout = 73;
    //const int minHeight = 1;
    const int maxHeight = 33;
    //float peakFloat = 1.2;
    float peak, peakVal;
    float peakPreM = 0.0;
    float peakPostM = 0.0;

    if (peakPre.available())
    {
        peakVal = peakPre.read();
        //peak = peakVal > 0.00065234f ? 20.0f*log10f(peakVal) : -65.0;  // Convert to dB and clip lower bounds just above noise floor
        peak = peakVal > 0.00065234f ? peakVal : 0.00065234f;
        peakPreM = mapf(peak, 0.0, 1.0, 1.0, 33.0);
        // Serial.print(peak, 8); Serial.print(" "); Serial.println(peakPreM);
        for (int i = 0; i < maxHeight; i++)
        {
            if (i < int(peakPreM))
            {
                im.blit(LITGREENVU, posXin, posYin - (i * 2), 1.0);
            }
        }
    }

    if (peakPost.available())
    {
        peakVal = peakPost.read();
        //peak = peakVal > 0.00065234f ? 20.0f*log10f(peakVal) : -65.0; // Convert to dB and clip lower bounds just above noise floor
        peak = peakVal > 0.00065234f ? peakVal : 0.00065234f;
        peakPostM = mapf(peak, 0, 1.0, 1.0, 33.0);
        for (int i = 0; i < maxHeight; i++)
        {
            if (i < int(peakPostM))
            {
                im.blit(LITGREENVU, posXout, posYout - (i * 2), 1.0);
            }
        }
    }


    //Draw current gain settings on VU meters
    // draw the IN gain graphics here...
    int y;
    y = map(myLineInLevel, 0, 15, 60, 2);
    im.drawCircle({15, (9 + y)}, 1, RGB565_Cyan);

    // draw the OUT gain graphics here...
    y = map(myLineOutLevel, 13, 31, 60, 2);
    im.drawCircle({305, (9 + y)}, 1, RGB565_Cyan);
}

void drawEQSliders()
{

    const int xSlider = 44 - 8;
    const int ySlider = 70;
    int y;

    for (int i = 0; i < 8; i++)
    {
        y = map(ydBLevel[i], -12, 12, 0, 64);
        im.blit(EQSlider, xSlider + (i * 22), ySlider - y, 1.0);
    }
}

void drawFFT()
{
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

    if (fftValues.available())
    {

        for (int i = 0; i < nBars-1; i++)  // was getting another FFT bar off the RT of the screen...
        {
            n = fftValues.read(fftOctTab[i * 2], fftOctTab[i * 2 + 1]);
            delta= fftOctTab[i * 2 + 1] - fftOctTab[i * 2];
            if (delta > 0)
            {
              n = n / delta;  // divide by number of bins in the sample to make average to graph
            } else {
              n=n*2; // edge case of first bin..
            }
            mVal = map( constrain(int(n),-80,0), -80, 0, 0, 36);
            //Serial.print(n,2); Serial.print(" "); Serial.println(mVal);
            int x = posX + i * barWidth;

            if (mVal >= barPeak[i])
            {
                barPeak[i] = mVal;
            }
            else
            {
                barPeak[i] = barPeak[i] - peakFloat;
            }
            for (int barNum = 0; barNum < maxHeight; barNum++)
            {
                if (barNum < mVal)
                {
                    im.blit(LITGREEN, x, posY - (barNum * 3), 1.0);
                }

                if (barNum == round(barPeak[i]))
                {
                    im.blit(LITWHITE, x, posY - (barNum * 3), 1.0);
                }
            }
        }
    }
}

void readFromFile()
{
    boolean they_match = true;

    EEPROM.get(0, tempMagic);
    for (int i = 0; i < 4; i++)
    {
        if (tempMagic[i] != EEPROM_MAGIC[i])
        {
            they_match = false;
            break;
        }
    }
    if (they_match)
    {
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

void deleteFile()
{
    EEPROM.put(0, EEPROM_INIT);
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
}
