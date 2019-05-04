#pragma once

#include "pattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class ConfettiPattern : public Pattern {
public:
    using Pattern::Pattern;

    const char *name() const override
    {
        return "confetti";
    }

    void run() override
    {
        fadeToBlackBy(&leds[0], leds.size(), 10);
        int pos = random16(leds.size());
        leds[pos] += CHSV(gHue++ + random8(64), 200, 255);
    }

private:
    uint8_t gHue = 0;
};
