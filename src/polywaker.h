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
    WakeMusic music;
    WakeLight light;
#if ENABLE_WAKE_COFFEE
    WakeCoffee coffee;
#endif

public:
    void setup()
    {
        ModeWaker::setup();

#if ENABLE_WAKE_LIGHT
        light.control(0);
#endif
#if ENABLE_WAKE_MUSIC
        music.control(0);
#endif
#if ENABLE_WAKE_COFFEE
        coffee.reset();
#endif
#if ENABLE_WAKE_BACKUP
        wake_backup_setup();
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
            light.control(0);
#endif
#if ENABLE_WAKE_MUSIC
            music.control(0);
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
            light.control(0);
            music.control(0);
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

#if 1 // auto set on double click
        if (since_click_arcade < 1000)
            setAlarmRelativeIn(2);
#endif
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
            light.control(light.get() + 0.05 * float(steps));
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
        coffee.loop(); // for auto turn off
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
            float gain = 1.0 - float(since_mode_changed()) / float(2000);

#if ENABLE_WAKE_LIGHT
            light.set(light.get() * constrain(gain, 0.0, 1.0));
#endif
#if ENABLE_WAKE_MUSIC
            music.set(music.get() * constrain(gain, 0.0, 1.0));
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
            if (coffee.is_OK_to_use())
            coffee.set(1);
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
            light.setRitualStep();
            light.set(percentage);
        }
#endif
#if ENABLE_WAKE_MUSIC
        wake_start = config.music_start * MIN_TO_MS;
        if (time > wake_start)
        {
            float percentage = float(time - wake_start) / float(config.music_end * MIN_TO_MS);
            music.set(percentage);
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