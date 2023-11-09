#ifndef WAKE_MUSIC_H
#define WAKE_MUSIC_H

#include <Arduino.h>
#include "config.h"
#include "util/http.h"

float control_music = 0.0;

bool radio_playing = false;

void radioplay()
{
    if (radio_playing)
        return;

    radio_playing = true;
    send_http("/radioplay");
}

void radiostop()
{
    if (!radio_playing)
        return;

    radio_playing = false;
    send_http("/radiostop");
}

void setVolume(float vol) // 0 - 1.0
{
    static int last_vol = -1;

    vol = constrain(vol, 0.0f, 1.0f);
    control_music = vol;

    // Convert to integer representation
    float vol_adj = mapConstrainf(vol, 0.0f, 1.0f, 0.1f, 0.6f);
    int i_vol = int(vol_adj * 100.0f);

    if (i_vol != last_vol)
    {
        last_vol = i_vol;
        send_http("/volume?volume=" + String(i_vol));
    }
}

//TODO everywhere make difference between setMusic / controlMusic ; setLight / controlLight better

// sends volume and controls radioplay/radiostop depening on value
void controlMusic(float value)
{
    if ((fabs(control_music - value) > 0.01) || ((value == 0.0) && (control_music != 0)))
    {
        setVolume(value);

        if (value > 0.0)
            radioplay();
        if (value == 0)
            radiostop();

        Serial.println("setMusic(" + String(value) + ")");
    }
}

void setMusic(float value)
{
    if ((control_music < 1) && (value >= 1)) // when value becomes full volume 1.0 
    {
        send_http("/random_message");
        Serial.println("Triggered /random_message !");
        control_music = value;
    }
    else
    {
        controlMusic(value);
    }
}

#endif // WAKE_MUSIC_H