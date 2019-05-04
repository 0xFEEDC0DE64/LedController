#pragma once

#include "pattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class SineleonPattern : public Pattern {
public:
    using Pattern::Pattern;

    const char *name() const override
    {
        return "sineleon";
    }

    void run() override
    {
        fadeToBlackBy(&leds[0], leds.size(), 20);
        int pos = beatsin16(13, 0, leds.size() - 1);
        leds[pos] += CHSV(gHue++, 255, 192);
    }

private:
    uint8_t gHue = 0;
};
