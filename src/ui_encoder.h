#pragma once
#ifndef UI_ENCODER_H
#define UI_ENCODER_H

#include "config.h"
#include "AiEsp32RotaryEncoder.h"

#define ROTARY_ENCODER_STEPS 4

class UI_Encoder : public AiEsp32RotaryEncoder
{
#define MAX_STEPS 100
public:
    UI_Encoder(int pinA, int pinB, int pinButton, int steps = ROTARY_ENCODER_STEPS) : AiEsp32RotaryEncoder(pinA, pinB, pinButton, -1, steps)
    {
        pinMode(pinA, INPUT_PULLUP);
        pinMode(pinB, INPUT_PULLUP);
    }

    void setup(void (*readEncoderISR)(void))
    {
        AiEsp32RotaryEncoder::areEncoderPinsPulldownforEsp32 = false;
        AiEsp32RotaryEncoder::begin();
        AiEsp32RotaryEncoder::setup(readEncoderISR);
        AiEsp32RotaryEncoder::setBoundaries(0, MAX_STEPS, true); // minValue, maxValue, circleValues true|false (when max go to min and vice versa)
        AiEsp32RotaryEncoder::disableAcceleration();
    }

    long encoderChanged()
    {
        return wrap((int)AiEsp32RotaryEncoder::encoderChanged(), -MAX_STEPS/2, MAX_STEPS/2);
    }
};

#endif // UI_ENCODER_H
