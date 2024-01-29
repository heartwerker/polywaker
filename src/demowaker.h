#pragma once
#ifndef ALARM_TIME_H
#define ALARM_TIME_H

#include "actual_time.h"
#include <elapsedMillis.h>
#include "ui_server_utils.h"

class Waker
{
    void setAlarm(Time time);
    void setAlarmRelativeIn(int sec);
    void setAlarmFromConfig();
    Time alarm();

    bool started();
    int secondsToAlarm();

    void stop();
    void snooze();
 
    int snoozeTime();
    int adjustSnoozeTime(int sec);
    int secondsToSnoozeOver();

    void setEnabled(bool e);
    void toggleEnabled();
    bool enabled();

[...]

};

public:
    elapsedMillis since_alarm_started = 0;
    elapsedMillis since_snooze_started = 0;
    elapsedMillis since_alarm_changed = 0;
    elapsedMillis since_enabled_changed = 0;

    bool _ringing = false;
    bool isSnoozing = false;

private:
    Time _tm_alarm;
    bool _enabled = false;
    int _untilAlarm = -1;

    int _snooze_time_s;
};

#endif // ALARM_TIME_H