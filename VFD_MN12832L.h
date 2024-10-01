/*
Mario Becker, 2018, License:MIT.

Code is for TeensyLC, others not tested! Critical things used:
- IntervalTimer
*/

#pragma once

#include <Adafruit_GFX.h>
#include <SPI.h>

template <size_t BitDepth> class MN12832Lgeneric : public Adafruit_GFX
{
public:
    MN12832Lgeneric(
        byte pinBLK = 4,
        byte pinLAT = 6,
        byte pinGCP = 3,
        byte MOSI_PIN = 11,
        byte SCK_PIN = 13,
        byte pinPWM = 20);
    ~MN12832Lgeneric(void);
    void begin();

    uint32_t getDisplayTime(); // 24us
    uint32_t getDisplayFps1(); // 24us

    virtual void drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void fillScreen(uint8_t color);
    void swapBuffers();

    /** must be called from timer or otherwise very regularly ! Go for min 4000 times per sec */
    static void refresh();

    static const int16_t targetFps = 150 * 44; // this is per gate // 

protected:
    static void nextGate();
    static const int16_t bufferSize = 136 / 8 * 32; // black'n'white, + margin

    // bufferlayout
    ///   front | back
    ///   0 | 1 | 0 | 1 //bitplanes

    uint8_t buffer[bufferSize*2*BitDepth];
    uint16_t bufferOffset = 0;
    uint8_t tempBuffer[30];

    union u32u4
    {
        uint64_t u32;
        uint8_t u4[4];
    };

    union u64u8
    {
        uint64_t u64;
        uint8_t u8[8];
    };

    uint8_t  gate;
    u64u8 gateBuf;

    uint32_t displayTime;
    uint32_t displayLast;
    uint32_t displayFps1;

    uint32_t loadLast;
    uint32_t loadFps1;

    const byte pinBLK;
    const byte pinLAT;
    const byte pinGCP;
    const byte MOSI_PIN;
    const byte SCK_PIN;
    const byte pinPWM;

    SPISettings spiSettings;

    static MN12832Lgeneric *_the;
};

/**
 * MN12832L in "black'n'white or mono or 1 bit depth.
 * In this case the two serial inputs must be wired in parallel.
 * * The refresh function is faster -> less CPU load.
 * * Needs less memory.
*/
class MN12832Lmono : public MN12832Lgeneric<1>
{
public:
    MN12832Lmono(
        byte pinBLK = 4,
        byte pinLAT = 6,
        byte pinGCP = 3,
        byte MOSI_PIN = 11,
        byte SCK_PIN = 13,
        byte pinPWM = 20);
};

/**
 * MN12832L with 4 grey levels, colors 0-3.
 * In this case the SOUT1 must be connected to SIN2 so the two shift registers are chained S1->S2.
 * * 2x CPU load then the mono version
 * * Uses 1k more RAM
*/
class MN12832Lgrey : public MN12832Lgeneric<2>
{
public:
    MN12832Lgrey(
        byte pinBLK = 4,
        byte pinLAT = 6,
        byte pinGCP = 3,
        byte MOSI_PIN = 11,
        byte SCK_PIN = 13,
        byte pinPWM = 20);
};

#include "VFD_MN12832L.inl"
