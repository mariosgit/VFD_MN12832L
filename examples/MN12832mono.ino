#include <Arduino.h>
#include <VFD_MN12832L.h>
#include <mbLog.h>
#include <elapsedMillis.h>

// Create an IntervalTimer object
#ifdef ARDUINO_ARCH_STM32
HardwareTimer timer(TIM3);
#else // teensy ??
IntervalTimer timer;
#endif

MN12832Lmono display( 
/* pinBLK = */ 0,
/* pinLAT = */ 1,
/* pinGCP = */ 2,
/* MOSI_PIN = */ 11,
/* SCK_PIN = */ 13);

const uint8_t pinPWM = 6; // I use this to modulate the Fillament Voltage !

elapsedMillis checker = 0;
elapsedMillis logger = 0;
uint32_t frameCounter = 0;
uint32_t drawtime = 0;
int16_t textPos = 128;

#ifndef MYEMOJI // build system experiment.. ignore
#define MYEMOJI ":)"
#endif

void setup()
{
    display.begin();
    randomSeed(analogRead(0));

    // You need to trigger the refresh function regularly !
#ifdef ARDUINO_ARCH_STM32
    timer.setOverflow(1000000 / display.targetFps, MICROSEC_FORMAT);
    timer.attachInterrupt([](HardwareTimer *timer) { display.refresh(); });
#else
    timer.begin(display.refresh, 1000000 / display.targetFps); // starting slowly
#endif

    pinMode(pinPWM, OUTPUT);
    analogWrite(pinPWM, 128);
}

void loop()
{
    if(checker > 100)
    {
        checker = 0;
        uint32_t time = micros();

// #define DRAWBEBUG 1
#ifdef DRAWBEBUG

        const uint16_t fullCol = 1;
        // writes 128 pix
        // v1 ~ 1975us
        // v2 ~ 1223us
        display.fillScreen(0);
        display.drawLine(0,0,31,31,fullCol);
        display.drawLine(32,0,32+31,31,fullCol);
        display.drawLine(64,0,64+31,31,fullCol);
        display.drawLine(96,0,96+31,31,fullCol);
        
        // display.drawRect(1,1,4,4,1);
        // display.drawRect(7,7,4,4,1);
        // LOG <<"-----------" <<LOG.endl;
#else
        /// draw random EQ
        // v2 ~ 31ms

        display.fillScreen(0);
        display.drawLine(0,10,127,10, 1);
        display.setCursor(textPos,0);
        display.setTextColor(1);
        display.setTextWrap(false);
        display.print("subermajo ");
        display.print(MYEMOJI); // defined in .env

        textPos -= 3;
        if(textPos < -120) textPos = 128;

        for(int16_t x = 0; x < 120; x+=9)
        {
            int16_t hi = random(16);
            // display.fillRect(x,16,7,31,0);
            display.fillRect(x,32 - hi, 7, hi, 1);
        }
#endif

        display.swapBuffers();

        drawtime = micros() - time;
    }
    if(logger > 1000)
    {
        // display.fillScreen(0);
        logger = 0;
        LOG <<LOG.dec <<"gate time: " <<display.getDisplayTime() <<"us, possible fps:" <<1000000/display.getDisplayTime()/44 <<" current fps:" <<1000000/display.getDisplayFps1()/44 <<" draw:" <<drawtime <<"us" <<LOG.endl;
    }
}

