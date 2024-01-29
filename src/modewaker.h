#pragma once
#ifndef MODEWAKER_H
#define MODEWAKER_H

#include "waker.h"

typedef enum
{
    STARTUP = 0,
    IDLE,
    RINGING,
    SNOOZING,
    SETTING_ALARM_HOUR,
    SETTING_ALARM_MINUTE,
    MAX_MODES
} MODES;

class ModeWaker : public Waker
{
public:
    void setup()
    {
        Waker::setup();
        setMode(IDLE);
    }

    String modeName()
    {
        switch (current_mode)
        {
        case STARTUP:
            return "STARTUP";
        case IDLE:
            return "IDLE";
        case RINGING:
            return "RINGING";
        case SNOOZING:
            return "SNOOZING";
        case SETTING_ALARM_HOUR:
            return "SETTING_ALARM_HOUR";
        case SETTING_ALARM_MINUTE:
            return "SETTING_ALARM_MINUTE";
        default:
            return "UNKNOWN";
        }
    }

    inline int mode() { return current_mode; }

    void setMode(int newMode)
    {
        if (current_mode == newMode)
            return;

        if ((newMode == SNOOZING) && (config.snooze_time == 0))
        {
            Serial.println("Snooze Time is zero therfore snoozing is disabled!");
            return;
        }

        since_mode_change = 0;
        current_mode = newMode;
        Serial.printf("setMode(%d) ------- %s\n", newMode, modeName().c_str());
    }

    void cycleMode()
    {
        setMode(current_mode + 1);
        if (current_mode >= MAX_MODES)
            setMode(STARTUP + 1);
    }

    long since_mode_changed() { return since_mode_change; }

private:
    int current_mode = STARTUP;
    elapsedMillis since_mode_change = 0;
};

#endif // MODEWAKER_H