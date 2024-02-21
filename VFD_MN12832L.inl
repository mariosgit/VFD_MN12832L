/// A GFX 1-bit canvas context for graphics

// #include <mbLog.h>


// moved out for more compability with other platforms
// // Create an IntervalTimer object
// #ifdef __STM32F1__
// HardwareTimer timer(2);
// #else
// IntervalTimer myTimer;
// #endif

template <size_t BitDepth>
MN12832Lgeneric<BitDepth>* MN12832Lgeneric<BitDepth>::_the = nullptr;

template <size_t BitDepth>
MN12832Lgeneric<BitDepth>::MN12832Lgeneric(
    byte pinBLK,
    byte pinLAT,
    byte pinGCP,
    byte MOSI_PIN,
    byte SCK_PIN,
    byte pinPWM) : Adafruit_GFX(128, 32),
                   gate(50),
                   displayTime(0),
                   pinBLK(pinBLK),
                   pinLAT(pinLAT),
                   pinGCP(pinGCP),
                   MOSI_PIN(MOSI_PIN),
                   SCK_PIN(SCK_PIN),
                   pinPWM(pinPWM),
                   spiSettings(16000000, MSBFIRST, SPI_MODE0)
{
    _the = this;
}

template <size_t BitDepth>
MN12832Lgeneric<BitDepth>::~MN12832Lgeneric(void)
{
}

template <size_t BitDepth>
void MN12832Lgeneric<BitDepth>::begin()
{
    pinMode(pinBLK, OUTPUT);
    pinMode(pinGCP, OUTPUT);
    pinMode(pinLAT, OUTPUT);

    digitalWrite(pinBLK, HIGH);
    digitalWrite(pinLAT, LOW);
    digitalWrite(pinGCP, LOW);

    SPI.setMOSI(MOSI_PIN);
    SPI.setSCK(SCK_PIN);
    SPI.begin();

    pinMode(pinPWM, OUTPUT);
    analogWrite(pinPWM, 128); // 50% duty

    // display needs to cygle all the time !
    // myTimer.begin(refresh, 1000000 / targetFps); // starting slowly
}

template <size_t BitDepth>
uint32_t MN12832Lgeneric<BitDepth>::getDisplayTime()
{
    uint32_t result;
    noInterrupts();
    result = _the->displayTime;
    interrupts();
    return result;
}

template <size_t BitDepth>
uint32_t MN12832Lgeneric<BitDepth>::getDisplayFps1()
{
    uint32_t result;
    noInterrupts();
    result = _the->displayFps1;
    interrupts();
    return result;
}

template <size_t BitDepth>
void MN12832Lgeneric<BitDepth>::drawPixel(int16_t px, int16_t py, uint16_t color)
{
    if ((px < 0) || (py < 0) || (px >= _width) || (py >= _height))
        return;
    if(color > 3)
        return;

    // mem coordinates, allways 3 bytes = 4 lines = 24 pixel in one block
    register uint8_t gate = px / 6 + 1;
    register uint8_t pixl = px % 6;
    register uint8_t yblk = py / 4;
    register uint8_t yoff = py % 4;

    // choose pixels
    register u32u4 pixp;
    if (pixl == 0)      pixp.u32 = B100000; // a
    else if (pixl == 1) pixp.u32 = B001000; // b
    else if (pixl == 2) pixp.u32 = B000010; // c
    else if (pixl == 3) pixp.u32 = B000001; // d
    else if (pixl == 4) pixp.u32 = B000100; // e
    else if (pixl == 5) pixp.u32 = B010000; // f

    // write pixel 6packs 111111xx xxxxxxxx xxxxxxxx
    if (yoff == 0)
        pixp.u32 = pixp.u32 << 18;
    else if (yoff == 1)
        pixp.u32 = pixp.u32 << 12;
    else if (yoff == 2)
        pixp.u32 = pixp.u32 << 6;
    else if (yoff == 3)
        pixp.u32 = pixp.u32 << 0;

    if(BitDepth == 1) // mono
    {
        uint8_t *dst = buffer + bufferOffset + 24 * gate + yblk * 3;
        if(color)
        {
            dst[0] |= (pixp.u4[2]);
            dst[1] |= (pixp.u4[1]);
            dst[2] |= (pixp.u4[0]);
        }
        else
        {
            dst[0] &= ~(pixp.u4[2]);
            dst[1] &= ~(pixp.u4[1]);
            dst[2] &= ~(pixp.u4[0]);
        }
    }
    else // grey
    {
        if(color & 1) // bitplane 0
        {
            uint8_t *dst = buffer + bufferOffset + 24 * gate + yblk * 3;
            dst[0] &= ~(pixp.u4[2]);
            dst[1] &= ~(pixp.u4[1]);
            dst[2] &= ~(pixp.u4[0]);
            dst[0] |= (pixp.u4[2]);
            dst[1] |= (pixp.u4[1]);
            dst[2] |= (pixp.u4[0]);
        }
        if(color & 2) // bitplane 1
        {
            uint8_t *dst = buffer + bufferOffset + bufferSize + 24 * gate + yblk * 3;
            dst[0] &= ~(pixp.u4[2]);
            dst[1] &= ~(pixp.u4[1]);
            dst[2] &= ~(pixp.u4[0]);
            dst[0] |= (pixp.u4[2]);
            dst[1] |= (pixp.u4[1]);
            dst[2] |= (pixp.u4[0]);
        }
    }

    // LOG <<LOG.dec <<"x:" <<px <<"x" <<py <<" X+6:\tG:" <<gate <<":" <<LOG.bin <<fetch <<LOG.endl;


    // LOG << LOG.dec << "pixel: " << x << "," << y << " gate:" << gate << " pixl:" << pixl << " yblk:" << yblk << " yoff:" << yoff  ;
    // LOG <<" buf:" <<LOG.bin;
    // LOG <<" " <<buffer[24 * gate + yblk + 0];
    // LOG <<" " <<buffer[24 * gate + yblk + 1];
    // LOG <<" " <<buffer[24 * gate + yblk + 2];
    // LOG << LOG.endl;
}

template <size_t BitDepth>
void MN12832Lgeneric<BitDepth>::fillScreen(uint8_t color)
{
    if(BitDepth == 1)
    {
        memset(buffer+bufferOffset, color, bufferSize*BitDepth);
        return;
    }
    if(color > 3) return;
    memset(buffer+bufferOffset, 0x00, bufferSize*BitDepth);
    if(color&1)
    {
        uint8_t *dst = buffer + bufferOffset;
        memset(dst, 0xff, bufferSize);
    }
    if(color & 2)
    {
        uint8_t *dst = buffer + bufferOffset + bufferSize;
        memset(dst, 0xff, bufferSize);
    }

    // byte tempBuffer[24] = {
    //     B10000010, B00001000, B00100000, // 4 rows | a
    //     B00100000, B10000010, B00001000, // 4 rows | b
    //     B00001000, B00100000, B10000010, // 4 rows | c
    //     B00000100, B00010000, B01000001, // 4 rows | d
    //     B00010000, B01000001, B00000100, // 4 rows | e
    //     B01000001, B00000100, B00010000, // 4 rows | f
    //     0, 0, 0,
    //     0, 0, 0};

    // LOG << "mset buffer: " << LOG.hex << (uint32_t)buffer << ": " << LOG.bin << buffer[0] << buffer[1] << buffer[2] << buffer[3] << LOG.endl;
}

template <size_t BitDepth>
void MN12832Lgeneric<BitDepth>::swapBuffers()
{
    noInterrupts();
    if(bufferOffset > 0)
    {
        bufferOffset = 0;
    }
    else
    {
        bufferOffset = bufferSize*BitDepth;
    }
    interrupts();
}

template <size_t BitDepth>
void MN12832Lgeneric<BitDepth>::nextGate()
{
    uint8_t gate = _the->gate;

    uint8_t endstop = 43;
    if (gate > endstop)
        gate = 0;

    // shift 2 neighbouring gates down 44 bits
    // but shifter has 48 bits / 6 bytes, use 6 bytes / last 5 bits digits unuses !
    if (gate == 0)
        _the->gateBuf.u64 = 0x8000000000000000;
    if (gate == 1)
        _the->gateBuf.u64 = 0xC000000000000000;
    // else if (gate == endstop)
    //     _the->gateBuf = 0x8000000000100000;
    else
        _the->gateBuf.u64 = _the->gateBuf.u64 >> 1;

    _the->gate = gate + 1;

    // LOG <<"gate:" <<LOG.dec <<gate <<LOG.hex <<" gb:" <<_the->gateBuf <<LOG.endl;
}

template <size_t BitDepth>
void MN12832Lgeneric<BitDepth>::refresh()
{
    _the->displayFps1 = micros() - _the->displayLast;
    _the->displayLast = micros();
    uint32_t time = micros();

    uint8_t *buffer = nullptr;
    buffer = _the->buffer; // double buffer disabled
    if(_the->bufferOffset)
    {
        buffer = _the->buffer;
    }
    else
    {
        buffer = _the->buffer + bufferSize*BitDepth;
    }

    // LOG <<"draw buffer: " <<LOG.hex <<(uint32_t)ptr <<": " <<LOG.bin <<*ptr++ <<*ptr++ <<LOG.endl;

    // if (_the->gate % 2 == 1)
    //     return;

    // LOG <<_the->gate <<LOG.endl;

    int8_t mask = (_the->gate % 2 == 1) ? B01010101 : B10101010;  // mask off either abc or cde pixels

    digitalWrite(_the->pinGCP, HIGH);
    digitalWrite(_the->pinGCP, LOW);

    for(size_t i = 0; i < BitDepth; i++)
    {
        uint8_t *ptr = (buffer + bufferSize*i + 24 * (_the->gate/2 + 0));

        uint8_t *dst = _the->tempBuffer;
        for(int i = 0; i< 24; i++)
        {
            *dst++ = (*ptr++) & mask;
        }
        // shift out gates   // bits 192-236 are the gates..
        // LOG <<"gate:" <<LOG.dec <<_the->_gate <<LOG.hex <<" gb:\t" <<_the->gateBuf <<LOG.endl;
        ptr = _the->gateBuf.u8 + 7;
        // LOG <<LOG.hex <<"         :\t";
        for (int i = 0; i < 6; i++)
        {
            *dst++ = *ptr;
            ptr--;
        }

        // copy columns from display buffer !
        ptr = _the->tempBuffer;

        // LOG <<"data[0]:" <<LOG.bin <<*ptr <<LOG.endl;

        digitalWrite(_the->pinGCP, HIGH);
        digitalWrite(_the->pinGCP, LOW);

        SPI.beginTransaction(_the->spiSettings);
        // shift out 24 bytes = 192 bits = 32rows * 3depth * 2colomns
        SPI.transfer(ptr, 30);

        // LOG <<LOG.endl;
        SPI.endTransaction();

        digitalWrite(_the->pinBLK, HIGH);

        digitalWrite(_the->pinLAT, HIGH);
        digitalWrite(_the->pinLAT, LOW);

        digitalWrite(_the->pinBLK, LOW);
    }

    nextGate();

    _the->displayTime = micros() - time;
}


/************************************************************/


MN12832Lmono::MN12832Lmono(
    byte pinBLK,
    byte pinLAT,
    byte pinGCP,
    byte MOSI_PIN,
    byte SCK_PIN,
    byte pinPWM) : MN12832Lgeneric(
        pinBLK, pinLAT, pinGCP, MOSI_PIN, SCK_PIN, pinPWM)
    {}
MN12832Lgrey::MN12832Lgrey(
    byte pinBLK,
    byte pinLAT,
    byte pinGCP,
    byte MOSI_PIN,
    byte SCK_PIN,
    byte pinPWM) : MN12832Lgeneric(
        pinBLK, pinLAT, pinGCP, MOSI_PIN, SCK_PIN, pinPWM)
    {}
