#pragma once
#ifndef DISPLAY_H
#define DISPLAY_H

#include "main_waker.h"
#include "polywaker.h"
extern PolyWaker waker;

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 32);

#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include "display_alarm_icon.h"

#include "ui.h"

void display_setup()
{
    display.begin();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.clearDisplay();
    display.setRotation(2);

    display.setTextSize(1);
    display.setCursor(0, 2);
    display.printf("checking");
    display.setTextSize(2);
    display.setCursor(0, 17);
    display.printf("WiFi...");
    display.display();
}

void display_webserver_message()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 5);
    display.printf("Find me:");
    display.setCursor(0, 20);
    display.printf("http://waker.local");
    display.display();
}

bool isInverted = false;
void display_invert(bool i)
{
    if (isInverted == i)
        return;
    display.invertDisplay(i);
    isInverted = i;
}

void display_printInt(int x, int y, int value, int size = 1)
{
    display.setTextSize(size, size);
    display.setCursor(x, y);
    display.printf("%d", value);
}

void display_time(int x, int y, Time time)
{
    display.setTextSize(1);

    y += 20;
    display.setCursor(x, y);
    display.setFont(&FreeSans18pt7b);
    display.printf("%02d", time.tm_hour);

    x += 42;
    display.setFont(&FreeSans9pt7b);
    if (millis() / 1000 % 2)
        for (int j = 0; j < 4; j++)
        {
            if (j >= (millis() / 250 % 4))
            {
                display.setCursor(x + j / 2, y + j % 2 - 9);
                display.printf(":");
            }
        }

    x += 10;
    display.setCursor(x, y);
    display.setFont(&FreeSans18pt7b);
    display.printf("%02d", time.tm_min);
}

void display_alarm_time(int x, int y, Time time)
{
    display.setTextSize(1);

    y += 20;
    display.setCursor(x, y);
    display.setFont(&FreeSansBold18pt7b);

    if (waker.mode() == SETTING_ALARM_HOUR)
        if (millis() / 200 % 2)
            display.setFont(&FreeSans18pt7b);

    display.printf("%02d", time.tm_hour);

    x += 42;
    display.setFont(&FreeSans9pt7b);
    for (int j = 0; j < 2; j++)
        for (int k = 0; k < 2; k++)
        {
            display.setCursor(x + j, y + k - 9);
            display.printf(":");
        }

    x += 10;
    display.setCursor(x, y);
    display.setFont(&FreeSansBold18pt7b);

    if (waker.mode() == SETTING_ALARM_MINUTE)
        if (millis() / 200 % 2)
            display.setFont(&FreeSans18pt7b);
    display.printf("%02d", time.tm_min);

}

void display_loop()
{
    static elapsedMillis since_render = 0;

    if (since_render < 40)
        return;

    display.clearDisplay();
    display_invert(false);

    since_render = 0;
    Time now = actual_time_get();

    if (ui.displaySleeping())
    {
        display.display(); // empty display
        return;
    }

    constexpr int x_time = 33;
    constexpr int y_time = 10;

    switch (waker.mode())
    {
    // ================================================
    case INIT:
    {
        break;
    }
    // ================================================
    case IDLE:
    {
        if (waker.since_alarm_changed < 2500)
            display_alarm_time(x_time, y_time, waker.alarm());
        else
            display_time(x_time, y_time, now);

        // show linear "fader" for light value
        if (ui.since_activity < 2000)
        {
            if (control_light > 0.01)
                for (int y = 0; y < 2; y++)
                    display.drawLine(0, y, 128 * control_light, y, 1);
                    
        }

        break;
    }
    // ================================================
    case RINGING:
    {
        if (config.blink_display_when_ringing)
            display_invert(!((millis() / 333) % 3));

        display_time(x_time, y_time, now);

        break;
    }
    case SNOOZING:
    {
        display.clearDisplay();

        display.setFont(&FreeSansBold9pt7b);
        display.setTextSize(1, 1);
        display.setCursor(0, 20);
        display.printf("%02d:%02d", now.tm_hour, now.tm_min);

        display.drawLine(40, 63, 60, 0, 1);

        display.setFont(&FreeSans9pt7b);
        display.setCursor(65, 30);
        int snooze_remaining = waker.snoozeRemaining();
        display.printf("%02d:%02d", snooze_remaining / 60, snooze_remaining % 60);

        break;
    }
    // ================================================
    case SETTING_ALARM_HOUR:
    case SETTING_ALARM_MINUTE:
    {
        display_alarm_time(x_time, y_time, waker.alarm());
        break;
    }
    }

    display.dim(ui.since_activity > 5000);

    // Overlay basic animations

    // TURN OFF 
    if (!waker.enabled() && waker.since_enabled_changed < 700)
    {
        display.clearDisplay();
        display.setFont(&FreeSans9pt7b);
        int move = waker.since_enabled_changed / 20;
        display.drawBitmap(0, 8 + move, alarmIcon_20_20, 20, 20, 1);
        display.setCursor(30, 25);
        display.printf("aus !");
    }
    // TURN ON 
    else if (waker.enabled() && waker.since_enabled_changed < 1500)
    {
        display.clearDisplay();
        display.setFont(&FreeSans9pt7b);
        int move = 400 / 20 - waker.since_enabled_changed / 10;
        move = constrain(move, 0, 1000 / 20);
        display.drawBitmap(0, 8 + move, alarmIcon_20_20, 20, 20, 1);

        display_alarm_time(x_time, y_time, waker.alarm());
    }
    // START SNOOZE
    else if (waker.mode() == SNOOZING)
    {
        if (waker.since_mode_changed() < 1300)
        {
            int move = 63 - waker.since_mode_changed() / 5;
            display.clearDisplay();
            display.setFont(&FreeSans9pt7b);
            display.setCursor(move, 20);
            display.printf("Snoo");

            for (int i = 0; i < 30; i++)
            {
                display.setCursor(move + (4 + i) * 10, 20);
                display.printf("z");
            }
        }
    }
    // BOUNCY CLOCK
    else if (waker.enabled())
    {
        if ((waker.mode() == RINGING) || 0)
        {
            float phasor = millis() / 1000.0 * 2 * PI;
            float x_factor = 0 + 0.5 * sinf(phasor);
            float y_factor = 1 - fabs(cosf(phasor));
            display.drawBitmap(0 + x_factor * 10, 3 + 8 * y_factor, alarmIcon_20_20, 20, 20, 1);

            if (waker._since_alarm_started > 1000)
            {
                float ritual_relative = float(waker._since_alarm_started) / float(30 / 2 * MIN_TO_MS);
                display.drawLine(0, 1, 128 * ritual_relative, 1, 1);
            }
        }
        else
            display.drawBitmap(0, 8, alarmIcon_20_20, 20, 20, 1);

    }

    display.display();
    // ================================================================================================
}

#endif // DISPLAY_H