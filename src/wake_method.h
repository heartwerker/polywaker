#pragma once

#include <Arduino.h>
#include "config.h"

class WakeMethod {
public:
    virtual ~WakeMethod() {}

    // control is meant for direct control of the output
    virtual void control(float value) = 0; 

    // set is meant for setting the target value (+ maybe other functionality)
    // set ultimately calls control
    virtual void set(float value) { control(value); }

    float get() { return _current; }

    float _current = 0.0; // in percent 0-1.

};
