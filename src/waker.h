#pragma once
#ifndef ALARM_TIME_H
#define ALARM_TIME_H

#include "actual_time.h"
#include <elapsedMillis.h>
#include "ui_server_utils.h"

class Waker
{
public:
    Waker() {}

    void setAlarmRelativeIn(int seconds)
    {
        struct tm timeinfo = actual_time_get();
        timeinfo.tm_sec += seconds;

        while (timeinfo.tm_sec >= 60)
        {
            timeinfo.tm_sec -= 60;
            timeinfo.tm_min += 1;
        }
        while (timeinfo.tm_min >= 60)
        {
            timeinfo.tm_min -= 60;
            timeinfo.tm_hour += 1;
        }
        if (timeinfo.tm_hour >= 24)
        {
            timeinfo.tm_hour = timeinfo.tm_hour % 24;
        }
        setAlarm(timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }

    void retrieveAlarmFromConfig()
    {
        setEnabled(config.alarm_enabled);
        _tm_alarm.tm_hour = config.alarm_hour;
        _tm_alarm.tm_min = config.alarm_minute;
        _tm_alarm.tm_sec = 0;

        _snooze_time_s = config.snooze_time * 60;

        since_alarm_changed = 0;
    }

    void setAlarm(int hour, int minute, int second)
    {
        _tm_alarm.tm_hour = (hour + 24) % 24;
        _tm_alarm.tm_min = (minute + 60) % 60;
        _tm_alarm.tm_sec = (second + 60) % 60;
        setAlarm(_tm_alarm);
    }

    void setAlarm(struct tm timeinfo)
    {
        _tm_alarm = timeinfo;
        setEnabled(true);

        // to server
        sendJson("alarm_hour", String(_tm_alarm.tm_hour));
        sendJson("alarm_minute", String(_tm_alarm.tm_min));

        config.alarm_enabled = _enabled;
        config.alarm_hour = _tm_alarm.tm_hour;
        config.alarm_minute = _tm_alarm.tm_min;
        config.save();
        Serial.println("Alarm time set to " + String(_tm_alarm.tm_hour) + ":" + String(_tm_alarm.tm_min));
    }

    struct tm getAlarmTime()
    {
        return _tm_alarm;
    }

    void stopAlarm()
    {
        setEnabled(false);
        isRinging = false;
        isSnoozing = false;
        num_snoozes = 0;
        _snooze_time_s = config.snooze_time * 60;
    }

    void snoozeAlarm()
    {
        since_snooze_started = 0;
        num_snoozes++;
        isRinging = false;
        isSnoozing = true;
        _snooze_time_s = config.snooze_time * 60;
    }

    bool alarmStarted() // has to be called at least with 1 Hz to not miss alarm
    {
        if (!_enabled)
        {
            return false;
        }

        if (!isSnoozing)
        {
            // detects last time was before alarm time now is after
            if ((_untilAlarm > 0) && (secondsToAlarm() <= 0))
            {
                if (!isRinging)
                {
                    isRinging = true;
                    num_alarm++;
                    since_alarm_started = 0;
                    return true;
                }
                else
                {
                    // already ringing
                    return false;
                }
            }
        }
        else if (isSnoozing)
        {
            if (since_snooze_started > (_snooze_time_s * 1000))
            {
                isRinging = true;
                since_alarm_started = 0;
                return true;
            }
        }

        _untilAlarm = secondsToAlarm();
        return false;
    }

    int snoozeTime() { return _snooze_time_s; }

    int adjustSnoozeTime(int seconds)
    {
        _snooze_time_s = constrain(_snooze_time_s + seconds, 1, 60 * 60);
        return _snooze_time_s;
    }

    int snoozeRemaining()
    {
        return (_snooze_time_s * 1000 - since_snooze_started) / 1000;
    }

    int secondsToAlarm()
    {
        if (!_enabled)
        {
            return 0;
        }
        struct tm timeinfo = actual_time_get();
        int seconds = 0;
        seconds += (_tm_alarm.tm_hour - timeinfo.tm_hour) * 60 * 60;
        seconds += (_tm_alarm.tm_min - timeinfo.tm_min) * 60;
        seconds += (_tm_alarm.tm_sec - timeinfo.tm_sec);
        return seconds;
    }

    int num_alarm = 0;
    int num_snoozes = 0;

    bool isEnabled()
    {
        return _enabled;
    }
    void setEnabled(bool e)
    {
        if (e != _enabled)
        {
            this->_enabled = e;
            since_enabled_changed = 0;
            sendJson("alarm_enabled", String(_enabled));
        }
    }
    void toggleEnabled()
    {
        setEnabled(!isEnabled());
    }


public:
    elapsedMillis since_alarm_started = 0;
    elapsedMillis since_snooze_started = 0;
    elapsedMillis since_alarm_changed = 0;
    elapsedMillis since_enabled_changed = 0;


    bool isRinging = false;
    bool isSnoozing = false;

private:
    struct tm _tm_alarm;
    bool _enabled = false;
    int _untilAlarm = -1;

    int _snooze_time_s;
};

#endif // ALARM_TIME_H