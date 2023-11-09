#pragma once
#ifndef DISPLAY_H
#define DISPLAY_H

#include "modes.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
Adafruit_SSD1306 display(128, 32);

#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

#include "display_alarm_icon.h"
extern Waker waker;

#include "ui.h"
extern UI ui;

void display_setup()
{
    display.begin();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.clearDisplay();
    display.setRotation(2);

#if 0
    display.fillScreen(WHITE);
    display.display();
    delay(10000000);
#endif
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

void display_time(int x, int y, int hour, int minute, int second)
{

#if 0
    y += 7;
    display.setTextSize(1);
    display.setCursor(x, y);
    display.printf("%02d:%02d:%02d", hour, minute, second);
#elif 0

    display.setTextSize(3, 3);
    display.setCursor(x, y);
    display.printf("%02d", hour);
    x += 32;
    display.setCursor(x, y);
    display.printf(":");
    x += 14;
    display.setCursor(x, y);
    display.printf("%02d", minute);
    x += 32;
    y += 7;
    display.setTextSize(2, 2);
    display.setCursor(x, y);
    display.printf(":%02d", second);
#else
    display.setTextSize(1);

    y += 20;
    display.setCursor(x, y);
    display.setFont(&FreeSans18pt7b);
    display.printf("%02d", hour);

    x += 42;
    display.setFont(&FreeSans9pt7b);
#if 1
    if (millis() / 1000 % 2)
        for (int j = 0; j < 4; j++)
        {
            if (j >= (millis() / 250 % 4))
            {
                display.setCursor(x + j / 2, y + j % 2 - 9);
                display.printf(":");
            }
        }
#else
    if (millis() / 1000 % 2)
        for (int j = 0; j < 6; j++)
        {
            if (j >= (millis() / int(1000.0 / 6.0) % 6))
            {
                display.setCursor(x + j % 2, y + j / 3 - 9);
                display.printf(":");
            }
        }

#endif

    x += 10;
    display.setCursor(x, y);
    display.setFont(&FreeSans18pt7b);
    display.printf("%02d", minute);

#endif
}

void display_alarm_time(int x, int y, int hour, int minute)
{
#if 0
    display.setTextSize(3, 3);
    display.setCursor(x, y);
    display.printf("%02d", hour);
    x += 32;
    display.setCursor(x, y);
    display.printf(":");
    x += 14;
    display.setCursor(x, y);
    display.printf("%02d", minute);
#else
    display.setTextSize(1);

    y += 20;
    display.setCursor(x, y);
    display.setFont(&FreeSansBold18pt7b);

    if (mode() == SETTING_ALARM_HOUR)
        if (millis() / 200 % 2)
            display.setFont(&FreeSans18pt7b);

    display.printf("%02d", hour);

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

    if (mode() == SETTING_ALARM_MINUTE)
        if (millis() / 200 % 2)
            display.setFont(&FreeSans18pt7b);
    display.printf("%02d", minute);

#endif
}

elapsedMillis since_render = 0;
void display_loop()
{
    if (since_render < 40)
        return;

    display.clearDisplay();
    display_invert(false);

    since_render = 0;
    struct tm time = actual_time_get();

    if (time.tm_hour < 8 || time.tm_hour > 22)
    {
        if (ui.displaySleeping)
        {
            display.display(); // empty display
            return;
        }
    }

#define TIME_X 33

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
#if 0
        display.drawLine(120, 31, 120, 32 - float(31) * volume, 1);

        if (waker.isRinging)
            display.drawChar(0, 0, 'A', 1, 1, 1);
        else
            display.drawChar(0, 0, '.', 1, 1, 1);

        display_printInt(105, 0, waker.secondsToAlarm(), 1);
#endif

        if (waker.since_alarm_changed < 2500)
        {
            tm alarm_time = waker.getAlarmTime();
            display_alarm_time(TIME_X, 10, alarm_time.tm_hour, alarm_time.tm_min);
        }
        else
            display_time(TIME_X, 10, time.tm_hour, time.tm_min, time.tm_sec);

        break;
    }
    // ================================================
    case RINGING:
    {
        display_invert(!((millis() / 333) % 3));
        display_time(TIME_X, 10, time.tm_hour, time.tm_min, time.tm_sec);

        break;
    }
    case SNOOZING:
    {
        display.clearDisplay();

        display.setFont(&FreeSansBold9pt7b);
        display.setTextSize(1, 1);
        display.setCursor(0, 20);
        display.printf("%02d:%02d", time.tm_hour, time.tm_min);

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
        tm alarm_time = waker.getAlarmTime();
        display_alarm_time(TIME_X, 10, alarm_time.tm_hour, alarm_time.tm_min);

        break;
    }
    }

#if 0
    tm now = actual_time_get();
    if (now.tm_hour >= waker.getAlarmTime().tm_hour &&
        now.tm_hour <= 23)
    {
        if (ui.since_activity > (60 * 1000))
            display.dim(true);
    }
    if (ui.since_activity < (500))
        display.dim(false);

    if (ui.since_activity > 2000)
#endif

    display.dim(ui.since_activity > 5000);

    if (!waker.isEnabled() && waker.since_enabled_changed < 700)
    {
        display.clearDisplay();
        display.setFont(&FreeSans9pt7b);
        int move = waker.since_enabled_changed / 20;
        display.drawBitmap(0, 8 + move, alarmIcon_20_20, 20, 20, 1);
        display.setCursor(30, 25);
        display.printf("aus !");
    }
    else if (waker.isEnabled() && waker.since_enabled_changed < 1500)
    {
        display.clearDisplay();
        display.setFont(&FreeSans9pt7b);
        int move = 400 / 20 - waker.since_enabled_changed / 10;
        move = constrain(move, 0, 1000 / 20);
        display.drawBitmap(0, 8 + move, alarmIcon_20_20, 20, 20, 1);

        tm alarm_time = waker.getAlarmTime();
        display_alarm_time(TIME_X, 10, alarm_time.tm_hour, alarm_time.tm_min);
    }
    else if (mode() == SNOOZING)
    {
        if (since_mode_change < 1300)
        {
            int move = 63 - since_mode_change / 5;
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
    else if (waker.isEnabled())
    {
        // display.drawBitmap(0, 2, alarmIcon_30_30, 30, 30, 1);
        if ((mode() == RINGING) || 0)
        {
            float phasor = millis() / 1000.0 * 2 * PI;
            float x_factor = 1 + 0.5 * sinf(phasor);
            float y_factor = 1 - fabs(cosf(phasor));
            display.drawBitmap(x_factor * 8, 8 * y_factor, alarmIcon_20_20, 20, 20, 1);
        }
        else
            display.drawBitmap(0, 8, alarmIcon_20_20, 20, 20, 1);
    }

    display.display();
    // ================================================================================================
}

#endif // DISPLAY_H