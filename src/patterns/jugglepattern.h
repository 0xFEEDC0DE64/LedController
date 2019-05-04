#pragma once

#include "pattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class JugglePattern : public Pattern {
public:
    using Pattern::Pattern;

    const char *name() const override
    {
        return "juggle";
    }

    void run() override
    {
        fadeToBlackBy(&leds[0], leds.size(), 20);
        byte dothue = 0;
        for(int i = 0; i < 8; i++) {
            leds[beatsin16(i+7, 0, leds.size() - 1)] |= CHSV(dothue, 200, 255);
            dothue += 32;
        }
    }
};
