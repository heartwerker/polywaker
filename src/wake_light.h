#ifndef WAKE_LIGHT_H
#define WAKE_LIGHT_H

#include <Arduino.h>
#include "config.h"

#include <elapsedMillis.h>

#if ENABLE_ESPNOW
#include "util/espnow.h"

int counter_printLight = 10000;

message_generic msg_light;
int led_retry_count = 0;
float control_light = 0.0;
elapsedMillis since_last_light = 0;


void sendLight(int index, float value) // 0.0 - 1.0
{
    msg_light.index = index;
    msg_light.value = constrain(value, 0, 1);
    esp_now_send(MAC_ADDRESS_LIGHT, (uint8_t *)&msg_light, sizeof(msg_light));
    // Serial.println("sendLight(" + String(msg_light.value) + ")");
}

// should be called slowly (from server command)
void controlLight(float value)
{
    control_light = constrain(value, 0.0, 1.0);

#if USER_IS_LEO
        sendLight(0, mapConstrainf(value, 0.0, 0.8, 0, 1));
        sendLight(1, mapConstrainf(value, 0.2, 1.0, 0, 1));
        for (int l = 2; l < 4; l++)
        {
            sendLight(l, value);
            delayMicroseconds(1000);
        }
#elif USER_IS_JANEK
        for (int l = 0; l < 4; l++)
        {
            sendLight(l, value);
            delayMicroseconds(1000);
        }
#elif USER_IS_DAVE
        // map value to W, R, G channels
        delayMicroseconds(1000);
        sendLight(0, (value - 0.2) / 0.8);
        delayMicroseconds(1000);
        sendLight(1, value * 5);                    // red
        delayMicroseconds(1000);
        sendLight(2, constrain(value * 2, 0, 0.7)); // green

#define offset 0.8
        if (value > offset)
        {
            delayMicroseconds(1000);
            sendLight(11, (value - offset) / (1 - offset) +0.2); // motor
        }
#endif
        since_last_light = 0;
}

// can be called constantly from loop
void setLight(float value) // 0.0 - 1.0
{
    if (since_last_light < 30)
        return;

    //Serial.println("setLight(" + String(msg_light.value) + ")");
    value = constrain(value, 0.0, 1.0);

    if (control_light == value)
        led_retry_count++;
    else
        led_retry_count = 0;

    if (led_retry_count < 3)
    {
        controlLight(value);

        if (counter_printLight++ > 50)
        {
            counter_printLight = 0;
            Serial.println("setLight(" + String(msg_light.value) + ")");
        }
    }
}

#endif

#endif // WAKE_LIGHT_H