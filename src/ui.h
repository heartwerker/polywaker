#pragma once
#ifndef UI_H
#define UI_H

#include "config.h"
#include "ui_encoder.h"
#include <OneButton.h>
#include <elapsedMillis.h>

class UI
{
public:
    UI(){};

    UI_Encoder rotaryEncoder = UI_Encoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN);
    OneButton rotaryButton = OneButton(ROTARY_ENCODER_BUTTON_PIN, true);
    OneButton arcadeButton = OneButton(BUTTON_ARCADE, true);

    void setup();
    void loop();

    long encoderChanged();

    elapsedMillis since_activity = 0;
    bool displaySleeping = false;
};

UI ui; // global object declaration

void IRAM_ATTR readEncoderISR() { ui.rotaryEncoder.readEncoder_ISR(); }

void UI::setup()
{
    rotaryEncoder.setup(readEncoderISR);
    arcadeButton.setDebounceTicks(20);
    arcadeButton.setPressTicks(1500); // 1.5s for long press

    // arcadeButton.setClickTicks(400);
    //  rotaryButton.setClickTicks(200);
}

void UI::loop()
{
    if (since_activity > (config.display_sleep_time_s * 1000))
        displaySleeping = true;
    if (mode() == RINGING)
        displaySleeping = false;

    rotaryButton.tick();
    arcadeButton.tick();

#if 0
    if (rotaryEncoder.encoderChanged())
    {
        Serial.println(rotaryEncoder.readEncoder());
    }
#endif
}

long UI::encoderChanged()
{
    long change = rotaryEncoder.encoderChanged();

    if (fabs(change))
        since_activity = 0;

    return change;
}

#endif // UI_H
