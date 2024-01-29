#pragma once
#ifndef UI_H
#define UI_H

#include "config.h"
#include "ui_encoder.h"
#include <OneButton.h>
#include <elapsedMillis.h>

void wakeOrForward(void *callback);

class UI
{
public:
    static UI& getInstance()
    {
        static UI instance;
        return instance;
    }

    void setup();
    void loop();

    UI_Encoder rotaryEncoder = UI_Encoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN);
    OneButton encoderButton = OneButton(ROTARY_ENCODER_BUTTON_PIN, true);
    OneButton arcadeButton = OneButton(BUTTON_ARCADE, true);

    void attachClickEncoder(void (*cb)(void)) { encoderButton.attachClick(wakeOrForward, reinterpret_cast<void *>(cb)); }
    void attachClickArcade(void (*cb)(void)) { arcadeButton.attachClick(wakeOrForward, reinterpret_cast<void *>(cb)); }
    void attachHoldArcade(void (*cb)(void)) { arcadeButton.attachLongPressStart(wakeOrForward, reinterpret_cast<void *>(cb)); }

    long encoderChanged();

    elapsedMillis since_activity = 0;

    void setDisplaySleeping(bool value) { _displaySleeping = value; }
    bool displaySleeping() { return _displaySleeping; }

private:
    UI() {} // Private constructor to prevent instantiation
    UI(const UI&) = delete; // Delete copy constructor
    UI& operator=(const UI&) = delete; // Delete assignment operator
    
    bool _displaySleeping = false;
};

UI& ui = UI::getInstance(); // Global access to the UI instance

void IRAM_ATTR readEncoderISR() { ui.rotaryEncoder.readEncoder_ISR(); }

typedef void (*CallbackFunction)();

void wakeOrForward(void *callback)
{
    CallbackFunction func = reinterpret_cast<CallbackFunction>(callback);
    ui.since_activity = 0;
    if (ui.displaySleeping())
    {
        ui.setDisplaySleeping(false);
        Serial.println("Woke display");
        return;
    }
    if (func != nullptr)
        func();
}


void UI::setup()
{
    rotaryEncoder.setup(readEncoderISR);
    arcadeButton.setDebounceTicks(20);
    arcadeButton.setPressTicks(1500); // 1.5s for long press

    // arcadeButton.setClickTicks(400);
    // encoderButton.setClickTicks(200);
}

void UI::loop()
{
    encoderButton.tick();
    arcadeButton.tick();
}

long UI::encoderChanged()
{
    long change = rotaryEncoder.encoderChanged();

    if (fabs(change))
        since_activity = 0;

    return change;
}

#endif // UI_H
