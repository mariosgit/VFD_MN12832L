#include "VFD_MN12832L.h"

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
