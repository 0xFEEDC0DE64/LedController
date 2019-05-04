#pragma once

#include "ledcontroller.h"

class Pattern {
public:
    Pattern(LedContainer leds) :
        leds(leds)
    {
    }

    virtual const char *name() const = 0;
    virtual void run() = 0;

protected:
    LedContainer &leds;
};
