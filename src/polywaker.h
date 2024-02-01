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

#if ENABLE_WAKE_COFFEE 
#include "wake_coffee.h"
#endif

#if ENABLE_WAKE_BACKUP
#include "wake_backup.h"
#endif

#if ENABLE_UI
#include "ui.h"
#endif

/**
 * @class PolyWaker
 * @brief The PolyWaker extends ModeWaker with handling UI and all wake_* modules
 * Also the actual wake ritual is implemented here.
 */
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
        ui.setDisplaySleeping(false); // always wakeup when mode changes

        ModeWaker::setMode(newMode);
        
        if (newMode == RINGING)
        {
            if (mode() == IDLE)
                Serial.println("Alarm started!");
            else if (mode() == SNOOZING)
                Serial.println("Alarm restarted!");
            else
                Serial.println("unusual?! Alarm started from mode " + String(modeName()));

#if ENABLE_WAKE_LIGHT
            controlLight(0);
#endif
#if ENABLE_WAKE_MUSIC
            controlMusic(0);
#endif
#if ENABLE_COFFEE
            controlCoffee(0);
#endif
        }
    }

    void holdArcade()
    {
        Serial.println("holdArcade()");
        switch (mode())
        {
        case IDLE:
        {
#if ENABLE_COFFEE
            setCoffee(0);
#endif
            setLight(0);
            setMusic(0);
            break;
        }
        case RINGING:
        case SNOOZING:
        {
            setMode(IDLE);
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
            setMode(SNOOZING);
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
        // Serial.printf("reactToEncoder(%d)\n", steps);
        switch (mode())
        {
        case IDLE:
            controlLight(control_light + 0.05 * float(steps));
            break;
        case RINGING:
            _since_alarm_started += steps * 10 * 1000;
            break;
        case SNOOZING:
            adjustSnoozeTime(-steps * 60); // by 1 min/step
            break;
        case SETTING_ALARM_HOUR:
            moveAlarm(steps * 3600);
            break;
        case SETTING_ALARM_MINUTE:
            moveAlarm(steps * 60);
            break;
        }
    }

    void loop()
    {
        ModeWaker::loop();

#if ENABLE_WAKE_COFFEE
        coffee_loop(); // to automatically turn off coffee machine
#endif

        switch (mode())
        {
        case RINGING:
        {
            wake_ritual(_since_alarm_started);
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
            break;
        }
        };
    }

    void wake_ritual(long time) // in ms
    {
        float wake_start = 0;

#if ENABLE_WAKE_COFFEE
            if (time > (config.coffee_start * MIN_TO_MS))
            {
                // only turn on if has not been used in the last hour
                if (since_coffee_on > (60 * MIN_TO_MS))
                    setCoffee(true);
            }
#endif

#if ENABLE_WAKE_LIGHT
            wake_start = config.light_start * MIN_TO_MS;
            if (time > wake_start)
            {
                float percentage = float(time - wake_start) / float(config.light_end * MIN_TO_MS);

#if ENABLE_WAKE_BACKUP
                if (config.alarm_hour)
                    if (time > (config.backup_start * MIN_TO_MS))
                        percentage = float(millis() % 1000) / 1000.0;
#endif

                setLight(percentage, true);
            }
#endif
#if ENABLE_WAKE_MUSIC
            wake_start = config.music_start * MIN_TO_MS;
            if (time > wake_start)
            {
                float percentage = float(time - wake_start) / float(config.music_end * MIN_TO_MS);
                setMusic(percentage);
            }
#endif

#if ENABLE_WAKE_BACKUP
            static float volume = 0.0; // ?!
            wake_start = config.backup_start * MIN_TO_MS;
            if (time > wake_start)
            {
                volume = float(time - wake_start) / float(config.backup_fade_relative_s * 1000.0);
                wake_backup_setVolume(volume);

                long time_backup = time - wake_start;
                float frequency = 0.5;
                frequency += (float(time_backup) / (config.backup_fade_relative_s * 1000)) * 1;
                wake_backup_setFrequency(constrain(frequency, 0.1, 2.0));
            }
#endif

    }
};

#endif // POLYWAKER_H