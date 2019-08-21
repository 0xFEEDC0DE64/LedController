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
#include "patterns/plasmapattern.h"

FASTLED_USING_NAMESPACE

class RGBTest : public Pattern
{
public:
    using Pattern::Pattern;

    const char * name() const override
    {
        return "rgbtest";
    }

    void run() override
    {
        CRGB color;
        switch(m_index)
        {
        case 0:
        case 1:
            color = CRGB::Red;
            break;
        case 2:
            color = CRGB::Green;
            break;
        case 3:
            color = CRGB::Blue;
        }

        fill_solid(&leds[0], NUM_LEDS, color);

        EVERY_N_SECONDS(1)
        {
            m_index++;
            if (m_index >= 4)
                m_index = 0;
        }
    }

private:
    int m_index = 0;
};

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
        m_plasma(leds),
        m_rgbtest(leds),
        patterns { &m_rainbow, &m_rainbowWithGlitter, &m_confetti, &m_sineleon, &m_juggle, &m_bpm, &m_fire2012, &m_plasma, &m_rgbtest },
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
    PlasmaPattern m_plasma;
    RGBTest m_rgbtest;

public:
    using PatternContainer = std::array<Pattern*, 9>;
    const PatternContainer patterns;
    PatternContainer::const_iterator iter;
};
