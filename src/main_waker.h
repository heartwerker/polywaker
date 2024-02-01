
#ifndef MAIN_WAKER_H
#define MAIN_WAKER_H

/**
 * @file main_waker.h
 * @brief The main_waker module connects the user interface (UI) to the (poly)waker instance,
 * which handles the alarm clock logic.
 */

#include <Arduino.h>
#include "config.h"

#include "config_user.h"

#include "polywaker.h"
PolyWaker waker;

#if ENABLE_UI
#include "ui.h"
#endif


// ================================================================================================
void main_waker_setup()
{
    Serial.println("main_waker_setup()");
    waker.setup();

    waker.setup();

#if ENABLE_UI
    ui.setup();
    ui.attachClickEncoder   ([]() { waker.clickEncoder(); });
    ui.attachClickArcade    ([]() { waker.clickArcade(); });
    ui.attachHoldArcade     ([]() { waker.holdArcade(); });
#endif

    if (!SPIFFS.begin())
        Serial.println("SPIFFS could not initialize");
    config_setup();

    waker.setAlarmFromConfig();
    
#if ENABLE_AUTO_START
    waker.setAlarmRelativeIn(10);
#endif
}

// ================================================================================================
void main_waker_loop()
{
    static elapsedMillis since_loop_state_machine = 0;
    if (since_loop_state_machine >= 50)
    {
        since_loop_state_machine = 0;

        if (waker.mode() == IDLE && !ui.displaySleeping())
            if (ui.since_activity > (config.display_sleep_time_after_s * 1000))
            {
                Time now = actual_time_get();
                if (now.tm_hour >= config.display_sleep_when_after_h 
                || now.tm_hour < config.display_sleep_when_before_h)
                    ui.setDisplaySleeping(true);
            }

        if (ui.displaySleeping())
            if (ui.since_activity < 200)
                ui.setDisplaySleeping(false);

        long steps = ui.encoderChanged();
        if (steps != 0)
            waker.reactToEncoder(steps);

        waker.loop();

        // stop setting alarm after inactivity
        if ((waker.mode() == SETTING_ALARM_HOUR) || (waker.mode() == SETTING_ALARM_MINUTE))
            if (ui.since_activity > (config.setting_inactivity_s * 1000))
                waker.setMode(IDLE);
    };
}

#endif // MAIN_WAKER_H