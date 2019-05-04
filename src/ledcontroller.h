#pragma once

#include <array>
#include <functional>

#include <FastLED.h>

#include "ledsettings.h"
#include "patterns/rainbowpattern.h"
#include "patterns/rainbowwithglitterpattern.h"
#include "patterns/confettipattern.h"
#include "patterns/sineleonpattern.h"
#include "patterns/jugglepattern.h"
#include "patterns/bpmpattern.h"
#include "patterns/fire2012pattern.h"

FASTLED_USING_NAMESPACE

class LedController
{
public:
    LedController() :
        controller(FastLED.addLeds<WS2812, pin, RGB>(&leds[0], leds.size())),
        m_rainbow(leds),
        m_rainbowWithGlitter(leds),
        m_confetti(leds),
        m_sineleon(leds),
        m_juggle(leds),
        m_bpm(leds),
        m_fire2012(leds),
        patterns { &m_rainbow, &m_rainbowWithGlitter, &m_confetti, &m_sineleon, &m_juggle, &m_bpm, &m_fire2012 },
        iter(patterns.begin())
    {
        controller.setCorrection(TypicalLEDStrip);
    }

    void nextPattern()
    {
        iter++;
        if (iter == patterns.end())
            iter = patterns.begin();
    }

    void run()
    {
        (*iter)->run();
    }

    void poweroff()
    {
        fill_solid(&leds[0], NUM_LEDS, CRGB::Black);
    }

private:
    std::array<CRGB, NUM_LEDS> leds;
    CLEDController &controller;

    RainbowPattern m_rainbow;
    RainbowWithGlitterPattern m_rainbowWithGlitter;
    ConfettiPattern m_confetti;
    SineleonPattern m_sineleon;
    JugglePattern m_juggle;
    BpmPattern m_bpm;
    Fire2012Pattern m_fire2012;

public:
    const PatternContainer patterns;
    PatternContainer::const_iterator iter;
};
