#pragma once
#ifndef MODEWAKER_H
#define MODEWAKER_H

#include "waker.h"

typedef enum
{
    INIT = 0,
    IDLE,
    RINGING,
    SNOOZING,
    SETTING_ALARM_HOUR,
    SETTING_ALARM_MINUTE,
    MAX_MODES
} MODES;

/**
 * @class ModeWaker
 * @brief The ModeWaker extends Waker with all mode handling and state machine in loop
 */
class ModeWaker : public Waker
{
public:
    void setup()
    {
        Waker::setup();
        setMode(INIT);
    }

    void loop()
    {
        Waker::loop(); // unused ..

        static int remainingBefore = 0;
        int remainingNow = secondsToAlarm();

        switch (mode())
        {
        case INIT:
        {
            setMode(IDLE);
            break;
        }
        case SNOOZING:
        {
            if (snoozeRemaining() < 0)
                setMode(RINGING);
            break;
        }
        case IDLE:
        {
            // Detect if AlarmTime was passed!!!
            if (enabled())
                if (remainingBefore > 0 && remainingNow <= 0)
                    setMode(RINGING);
            break;
        }
        };

        remainingBefore = remainingNow;
    }
    // ================================================================
    void setMode(int new_mode)
    {
        if (current_mode == new_mode)
            return;

        switch (new_mode)
        {
        case INIT:
            break;
        case IDLE:
            setEnabled(false); // TODO Really always disable or keep for the next day ?!
            break;
        case RINGING:
            _since_alarm_started = 0;
            break;
        case SNOOZING:
            if (config.alarm_snooze_time == 0)
            {
                Serial.println("Snooze Time is zero -> No snoozing!");
                return;
            }
            Waker::snooze();
            break;
        };

        since_mode_change = 0;
        current_mode = new_mode;
        Serial.printf("setMode(%d) ------- %s\n", new_mode, modeName().c_str());
    }

    inline int mode() { return current_mode; }

    long since_mode_changed() { return since_mode_change; }

private:
    int current_mode = INIT;
    elapsedMillis since_mode_change = 0;

    // ================================================================
public:
    String modeName()
    {
        switch (current_mode)
        {
        case INIT:
            return "INIT";
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
};

#endif // MODEWAKER_H