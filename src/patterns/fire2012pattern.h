#pragma once

#include "pattern.h"

#include <FastLED.h>

#include "ledsettings.h"
#include "ledcontroller.h"

FASTLED_USING_NAMESPACE

class Fire2012Pattern : public Pattern
{
    static constexpr auto COOLING = 55;
    static constexpr auto SPARKING = 120;
    static constexpr auto gReverseDirection = false;

    std::array<byte, NUM_LEDS> heat;

public:
    using Pattern::Pattern;

    const char *name() const override
    {
        return "fire2012";
    }

    void run() override
    {
        // Step 1.  Cool down every cell a little
        for( int i = 0; i < leds.size(); i++)
        {
            heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / leds.size()) + 2));
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for( int k= leds.size() - 1; k >= 2; k--)
        {
            heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
        }

        // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
        if( random8() < SPARKING )
        {
            int y = random8(7);
            heat[y] = qadd8( heat[y], random8(160,255) );
        }

        // Step 4.  Map from heat cells to LED colors
        for( int j = 0; j < leds.size(); j++)
        {
            CRGB color = HeatColor( heat[j]);
            int pixelnumber;

            if( gReverseDirection )
                pixelnumber = (leds.size()-1) - j;
            else
                pixelnumber = j;

            leds[pixelnumber] = color;
        }
    }
};
