#pragma once

#include "pattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class BpmPattern : public Pattern {
public:
    using Pattern::Pattern;

    const char *name() const override
    {
        return "bpm";
    }

    void run() override
    {
        uint8_t BeatsPerMinute = 62;
        CRGBPalette16 palette = PartyColors_p;
        uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
        for(int i = 0; i < leds.size(); i++)
            leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-(gHue++)+(i*10));
    }

private:
    uint8_t gHue = 0;
};
