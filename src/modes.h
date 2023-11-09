#pragma once
#ifndef MODES_H
#define MODES_H

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

int current_mode = STARTUP;
elapsedMillis since_mode_change = 0;

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

    since_mode_change = 0;
    current_mode = newMode;
    Serial.println("setMode() ------- " + modeName());
}

void cycleMode()
{
    setMode(current_mode + 1);
    if (current_mode >= MAX_MODES)
        setMode(STARTUP + 1);
}

#endif // MODES_H