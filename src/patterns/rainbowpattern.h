#pragma once

#include "pattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class RainbowPattern : public Pattern {
public:
    using Pattern::Pattern;

    const char *name() const override
    {
        return "rainbow";
    }

    void run() override
    {
        fill_rainbow(&leds[0], leds.size(), gHue++, 7);
    }

private:
    uint8_t gHue = 0;
};
