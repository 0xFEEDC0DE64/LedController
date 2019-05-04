#pragma once

#include "rainbowpattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class RainbowWithGlitterPattern : public RainbowPattern {
public:
    using RainbowPattern::RainbowPattern;

    const char *name() const override
    {
        return "rainbowWithGlitter";
    }

    void run() override
    {
        RainbowPattern::run();
        addGlitter(80);
    }

private:
    void addGlitter(fract8 chanceOfGlitter)
    {
        if(random8() < chanceOfGlitter)
            leds[random16(leds.size())] += CRGB::White;
    }
};
