
#ifndef MAIN_WAKER_H
#define MAIN_WAKER_H

#include <Arduino.h>
#include "config.h"
#include "modes.h"
#include "actual_time.h"

#include "waker.h"
Waker waker;

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
extern UI ui;
#endif

float volume = 0.0;
elapsedMillis since_loop_state_machine = 0;

// ================================================================================================
#if ENABLE_UI

bool userActivityWokeDisplay()
{
    ui.since_activity = 0;
    if (ui.displaySleeping)
    {
        ui.displaySleeping = false;
        Serial.println("Woke display");
        return true;
    }
    return false;
}

void arcadeButtonLongPress()
{
    Serial.println("arcadeButtonLongPress()");
    if (userActivityWokeDisplay())
        return;

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
        waker.stopAlarm();
        setMode(IDLE);
#if 0
        setCoffee(false);
#endif
        break;
    }
    case SNOOZING:
    {
        waker.stopAlarm();
        setMode(IDLE);
#if 0
        setCoffee(false);
#endif
        break;
    }
    }
}
void arcadeButtonClicked()
{
    Serial.println("arcadeButtonClicked()");
    if (userActivityWokeDisplay())
        return;

    if (ui.displaySleeping)
    {
        ui.displaySleeping = false;
        return;
    }
    
    switch (mode())
    {
    case IDLE:
    {
        waker.toggleEnabled();
        break;
    }
    case SETTING_ALARM_HOUR:
    case SETTING_ALARM_MINUTE:
    {
        waker.setEnabled(true);
        setMode(IDLE);
        break;
    }
    case RINGING:
    {
        waker.snoozeAlarm();
        setMode(SNOOZING);
        break;
    }
    case SNOOZING:
    {
        if (!waker.isEnabled())
        {
            waker.setEnabled(true);
            setMode(IDLE);
        }
        break;
    }
    }
}

void rotaryButtonClicked()
{
    Serial.println("rotaryButtonClicked()");
    if (userActivityWokeDisplay())
        return;

    if (mode() == IDLE)
    {
        waker.setEnabled(true);
        setMode(SETTING_ALARM_MINUTE);
    }
    else if (mode() == SETTING_ALARM_MINUTE)
    {
        setMode(SETTING_ALARM_HOUR);
    }
    else if (mode() == SETTING_ALARM_HOUR)
        setMode(IDLE);

}
#endif

void main_waker_setup()
{
    Serial.println("main_waker_setup()");

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

void main_waker_loop()
{
    // ================================================================================================
    if (since_loop_state_machine >= 50)
    {
        coffee_loop(); // to automatically turn off coffee machine

        long encoder_change = ui.encoderChanged();

        if (fabs(encoder_change) && ui.displaySleeping)
        {
            ui.displaySleeping = false;
            ui.since_activity = 0;
            encoder_change = 0;
        }

        since_loop_state_machine = 0;
        switch (mode())
        {
        // ================================================
        case STARTUP:
        {
            break;
        }
        // ================================================
        case IDLE:
        {
            if (abs(encoder_change))
            {
#if 0
                setMode(SETTING_ALARM_MINUTE);
                waker.setEnabled(true);
#else
                controlLight(control_light + 0.05 * float(encoder_change));
#endif
            }

            if (waker.alarmStarted())
            {
                Serial.println("Alarm started!");
                setMode(RINGING);
            }
            break;
        }
        case RINGING:
        {
            if (ui.displaySleeping)
                ui.displaySleeping = false;

            if (!waker.isEnabled())
            {
                waker.stopAlarm();
                setMode(IDLE);
            }

            #if 1
            if (abs(encoder_change))
            {
                waker.since_alarm_started += encoder_change * 10 * 1000;
            }
            #endif

            if (!waker.isRinging)
                setMode(IDLE);
            else
            {
                long since_start = waker.since_alarm_started;
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
                    setLight(bri);
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
                    waker.stopAlarm();
                    setLight(0.0);
                    if (waker.num_alarm < 2)
                        waker.setAlarmRelativeIn(5);
                }
#endif
            }

            break;
        }
        // ================================================
        case SNOOZING:
        {
#if ENABLE_WAKE_LIGHT
            float bri = 1.0 - float(since_mode_change) / float(2000);
            bri = control_light * constrain(bri, 0.0, 1.0);
            setLight(bri);
#endif

#if ENABLE_WAKE_MUSIC
            float vol = 1.0 - float(since_mode_change) / float(2000);
            vol = control_music * constrain(vol, 0.0, 1.0);
            setMusic(vol);
#endif

            if (abs(encoder_change))
            {
                int snooze = waker.adjustSnoozeTime(encoder_change * 60);
                Serial.println("Snooze time: " + String(snooze));
            }

            if (waker.alarmStarted())
            {
                Serial.println("Alarm re-started!");
                setMode(RINGING);
            }
            break;
        }
        // ================================================
        case SETTING_ALARM_HOUR:
        {
            if (abs(encoder_change))
            {
                tm alarm = waker.getAlarmTime();
                waker.setAlarm(alarm.tm_hour + encoder_change, alarm.tm_min, 0);
            }

            if (ui.since_activity > (config.setting_inactivity_s * 1000))
                setMode(IDLE);

            break;
        }
        // ================================================
        case SETTING_ALARM_MINUTE:
        {
            if (abs(encoder_change))
            {
                tm alarm = waker.getAlarmTime();
                waker.setAlarm(alarm.tm_hour, alarm.tm_min + encoder_change, 0);
            }

            if (ui.since_activity > (config.setting_inactivity_s * 1000))
                setMode(IDLE);

            break;
        }
        }
        // ================================================================================================
    }
}

#endif // MAIN_WAKER_H