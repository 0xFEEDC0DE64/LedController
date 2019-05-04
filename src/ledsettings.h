#pragma once

#include <array>

#include <FastLED.h>

FASTLED_USING_NAMESPACE

class Pattern;

constexpr auto pin = 0;
constexpr auto NUM_LEDS = 100;
using LedContainer = std::array<CRGB, NUM_LEDS>;
using PatternContainer = std::array<Pattern*, 7>;
