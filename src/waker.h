#pragma once
#ifndef WAKER_H
#define WAKER_H

#include "actual_time.h"
#include <elapsedMillis.h>
#include "ui_server_utils.h"

class Waker
{
public:
    void setup()
    {
        actual_time_setup();
    }

    void setAlarmRelativeIn(int seconds)
    {
        Time now = actual_time_get();
        now.tm_sec += seconds;

        while (now.tm_sec >= 60)
        {
            now.tm_sec -= 60;
            now.tm_min += 1;
        }
        while (now.tm_min >= 60)
        {
            now.tm_min -= 60;
            now.tm_hour += 1;
        }

        now.tm_hour = now.tm_hour % 24;
        setAlarm(now, false);
    }

    void setAlarmFromConfig()
    {
        setEnabled(config.alarm_enabled);
        _alarm.tm_hour = config.alarm_hour;
        _alarm.tm_min = config.alarm_minute;
        _alarm.tm_sec = 0;

        _snooze_time_s = config.snooze_time * 60;

        since_alarm_changed = 0;
    }

    void moveAlarm(int seconds)
    {
        // Serial.println("moveAlarm(" + String(seconds) + ")");

        int hour = seconds / 3600;
        seconds -= hour * 3600;
        int minute = seconds / 60;
        seconds -= minute * 60;

        _alarm.tm_hour = (_alarm.tm_hour + hour + 24) % 24;
        _alarm.tm_min = (_alarm.tm_min + minute + 60) % 60;
        _alarm.tm_sec = seconds;
        setAlarm(_alarm);
    }

    void setAlarm(Time time, bool save=true)
    {
        _alarm = time;
        setEnabled(true);

        if (save)
        {
            // to server
            config.alarm_enabled = _enabled;
            config.alarm_hour = _alarm.tm_hour;
            config.alarm_minute = _alarm.tm_min;

            sendJson(config.alarm_hour);
            sendJson(config.alarm_minute);
            config.save();
        }
        Serial.println("Alarm time set to " + String(_alarm.tm_hour) + ":" + String(_alarm.tm_min));
    }

    Time alarm() { return _alarm; }

    void stopAlarm()
    {
        setEnabled(false);
        _ringing = false;
        _snoozing = false;
        num_snoozes = 0;
        _snooze_time_s = config.snooze_time * 60;
    }

    void snoozeAlarm()
    {
        since_snooze_started = 0;
        num_snoozes++;
        _ringing = false;
        _snoozing = true;
        _snooze_time_s = config.snooze_time * 60;
    }

    // TODO move this to statemachine - or move statemachine here.
    
    bool alarmStarted() // has to be called at least with 1 Hz to not miss alarm
    {
        if (!_enabled)
        {
            return false;
        }

        if (!_snoozing)
        {
            // detects last time was before alarm time now is after
            if ((_untilAlarm > 0) && (secondsToAlarm() <= 0))
            {
                if (!_ringing)
                {
                    
                    _ringing = true;
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
        else if (_snoozing)
        {
            if (since_snooze_started > (_snooze_time_s * 1000))
            {
                _ringing = true;
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
        Time now = actual_time_get();
        int seconds = 0;
        seconds += (_alarm.tm_hour - now.tm_hour) * 60 * 60;
        seconds += (_alarm.tm_min - now.tm_min) * 60;
        seconds += (_alarm.tm_sec - now.tm_sec);
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
            config.alarm_enabled = _enabled;
            sendJson(config.alarm_enabled);
        }
    }
    void toggleEnabled()
    {
        setEnabled(!isEnabled());
    }

    bool isRinging() { return _ringing; }
    bool isSnoozing() { return _snoozing; }

public:
    elapsedMillis since_alarm_started = 0;
    elapsedMillis since_snooze_started = 0;
    elapsedMillis since_alarm_changed = 0;
    elapsedMillis since_enabled_changed = 0;

private:
    bool _snoozing = false;
    bool _ringing = false;

    Time _alarm;
    bool _enabled = false;
    int _untilAlarm = -1;

    int _snooze_time_s;
};

#endif // WAKER_H