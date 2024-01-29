#pragma once
#ifndef POLYWAKER_H
#define POLYWAKER_H

#include "modewaker.h"

#if ENABLE_WAKE_LIGHT
#include "wake_light.h"
#endif

#if ENABLE_WAKE_MUSIC
#include "wake_music.h"
#endif

#if ENABLE_WAKE_COFFEE || 1 // why was this always included ?!
#include "wake_coffee.h"
#endif

#if ENABLE_WAKE_BACKUP
#include "wake_backup.h"
#endif

#if ENABLE_UI
#include "ui.h"
#endif

class PolyWaker : public ModeWaker
{
public:
    void setup()
    {
        ModeWaker::setup();

#if ENABLE_WAKE_BACKUP
    wake_backup_setup();
#endif

#if ENABLE_WAKE_LIGHT
        setLight(0.0);
#endif
#if ENABLE_WAKE_COFFEE
        coffee_reset();
#endif
#if ENABLE_WAKE_MUSIC
        setMusic(0);
#endif
    }

    void setMode(int newMode)
    {
        ModeWaker::setMode(newMode);
        ui.setDisplaySleeping(false);  // always wakeup when mode changes
    }

    void holdArcade()
    {
        Serial.println("holdArcade()");
        switch (mode())
        {
        case IDLE:
        {
            setCoffee(0);
            setLight(0);
            setMusic(0);
            break;
        }
        case RINGING:
        {
            stopAlarm();
            setMode(IDLE);
#if 0
        setCoffee(false);
#endif
            break;
        }
        case SNOOZING:
        {
            stopAlarm();
            setMode(IDLE);
#if 0
        setCoffee(false);
#endif
            break;
        }
        }
    }

    elapsedMillis since_click_arcade = 0;
    void clickArcade()
    {
        Serial.println("clickArcade()");
        // Serial.printf("since_click_arcade: %d\n", (long) since_click_arcade);

        switch (mode())
        {
        case IDLE:
        {
            toggleEnabled();
            break;
        }
        case SETTING_ALARM_HOUR:
        case SETTING_ALARM_MINUTE:
        {
            setEnabled(true);
            setMode(IDLE);
            break;
        }
        case RINGING:
        {
            snoozeAlarm();
            setMode(SNOOZING);
            break;
        }
        case SNOOZING:
        {
            if (!isEnabled())
            {
                setEnabled(true);
                setMode(IDLE);
            }
            break;
        }
        }

        if (since_click_arcade < 1000)
            setAlarmRelativeIn(2);
        since_click_arcade = 0;
    }

    void clickEncoder()
    {
        Serial.println("clickEncoder()");
        if (mode() == IDLE)
        {
            setEnabled(true);
            setMode(SETTING_ALARM_MINUTE);
        }
        else if (mode() == SETTING_ALARM_MINUTE)
        {
            setMode(SETTING_ALARM_HOUR);
        }
        else if (mode() == SETTING_ALARM_HOUR)
            setMode(IDLE);
    }

    void reactToEncoder(int steps)
    {
        switch (mode())
        {
        case IDLE:
            controlLight(control_light + 0.05 * float(steps));
            break;
        case RINGING:
            since_alarm_started += steps * 10 * 1000;
            break;
        case SNOOZING:
        {
            int snooze = adjustSnoozeTime(-steps * 60);
            Serial.println("Snooze time: " + String(snooze));
            break;
        }
        case SETTING_ALARM_HOUR:
            moveAlarm(steps * 3600);
            break;
        case SETTING_ALARM_MINUTE:
            moveAlarm(steps * 60);
            break;
        }
    }

    void loop_state_machine()
    {
        coffee_loop(); // to automatically turn off coffee machine

        switch (mode())
        {
        // ================================================
        case IDLE:
        {
            if (alarmStarted())
            {
                Serial.println("Alarm started!");
                controlLight(0);
                controlMusic(0);
#if ENABLE_COFFEE
                controlCoffee(0);
#endif
                // MAIN POINT OF STARTING ALARM.
                setMode(RINGING);
            }
            break;
        }
        case RINGING:
        {
            if (!isEnabled())
            {
                stopAlarm();
                setMode(IDLE);
            }

            if (!isRinging())
                setMode(IDLE);
            else
            {
                long since_start = since_alarm_started;
                float start;

#if ENABLE_WAKE_COFFEE
                if (since_start > (config.coffee_start * MIN_TO_MS))
                {
                    // only turn on if has not been used in the last hour
                    if (since_coffee_on > (60 * MIN_TO_MS))
                        setCoffee(true);
                }
#endif
#if ENABLE_WAKE_LIGHT
                start = config.light_start * MIN_TO_MS;
                if (since_start > start)
                {
                    float bri = float(since_start - start) / float(config.light_end * MIN_TO_MS);

#if 1 // blink light when backup is on
                    if (since_start > (config.backup_start * MIN_TO_MS))
                        bri = float(millis() % 1000) / 1000.0;
#endif
                    setLight(bri, true);
                }
#endif
#if ENABLE_WAKE_MUSIC
                start = config.music_start * MIN_TO_MS;
                if (since_start > start)
                {
                    float vol = float(since_start - start) / float(config.music_end * MIN_TO_MS);
                    setMusic(vol);
                }
#endif

#if ENABLE_WAKE_BACKUP

                static float volume = 0.0; // ?!
                start = config.backup_start * MIN_TO_MS;
                if (since_start > start)
                {
                    volume = float(since_start - start) / float(config.backup_fade_relative_s * 1000.0);
                    wake_backup_setVolume(volume);

                    long since_start_backup = since_start - start;
                    float frequency = 0.5;
                    frequency += (float(since_start_backup) / (config.backup_fade_relative_s * 1000)) * 1;
                    wake_backup_setFrequency(constrain(frequency, 0.1, 2.0));
                }
#endif

#if ENABLE_AUTO_RESTART
                if (since_start >= 3000)
                {
                    stopAlarm();
                    setLight(0.0);
                    if (num_alarm < 2)
                        setAlarmRelativeIn(5);
                }
#endif
            }

            break;
        }
        // ================================================
        case SNOOZING:
        {
#if ENABLE_WAKE_LIGHT
            float bri = 1.0 - float(since_mode_changed()) / float(2000);
            bri = control_light * constrain(bri, 0.0, 1.0);
            setLight(bri);
#endif

#if ENABLE_WAKE_MUSIC
            float vol = 1.0 - float(since_mode_changed()) / float(2000);
            vol = control_music * constrain(vol, 0.0, 1.0);
            setMusic(vol);
#endif

            if (alarmStarted())
            {
                Serial.println("Alarm re-started!");
                setMode(RINGING);
            }
            break;
        }
        };
    }
};

#endif // POLYWAKER_H