#pragma once

#include <array>
#include <functional>

#include <FastLED.h>

#include "patterns/rainbowpattern.h"
#include "patterns/rainbowwithglitterpattern.h"
#include "patterns/confettipattern.h"
#include "patterns/sineleonpattern.h"
#include "patterns/jugglepattern.h"
#include "patterns/bpmpattern.h"
#include "patterns/fire2012pattern.h"

class LedController {
public:

private:
    static constexpr auto NUM_LEDS = 100;

    std::array<CRGB, NUM_LEDS> leds;

    RainbowPattern m_rainbow;
    RainbowWithGlitterPattern m_rainbowWithGlitter;
    ConfettiPattern m_confetti;
    SineleonPattern m_sineleon;
    JugglePattern m_juggle;
    BpmPattern m_bpm;
    Fire2012Pattern m_fire2012;

    using PatternContainer = std::array<Pattern*, 7>;
};
