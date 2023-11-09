#ifndef WAKE_COFFEE_H
#define WAKE_COFFEE_H

#include <Arduino.h>
#include "config.h"

#include <elapsedMillis.h>

#if ENABLE_ESPNOW
#include "util/espnow.h"

bool control_coffee = false;
elapsedMillis since_coffee_on = 60 * MIN_TO_MS;

message_generic msg_coffee;
int coffee_retry_count = 0;

void controlCoffee(bool value)
{
    since_coffee_on = 60 * MIN_TO_MS;

    control_coffee = value;
    Serial.println("setCoffee(" + String(control_coffee) + ")");

    msg_coffee.value = !control_coffee;
    msg_coffee.index = 0;
    esp_now_send(MAC_ADDRESS_COFFEE, (uint8_t *)&msg_coffee, sizeof(msg_coffee));

}

void setCoffee(bool value)
{

    if (control_coffee == value)
        coffee_retry_count++;
    else
        coffee_retry_count = 0;

    if (coffee_retry_count < 3)
    {
        controlCoffee(value);

        if (control_coffee)
            since_coffee_on = 0;
    }
}

void coffee_reset()
{
    controlCoffee(true);
    controlCoffee(false);
    since_coffee_on = 60 * MIN_TO_MS;
}

void coffee_loop()
{
    if (control_coffee)
    {
        // if (since_coffee_on > (5000))
        // AUTO TURN OFF after 8 minutes
        if (since_coffee_on > (8 * MIN_TO_MS))
        {
            setCoffee(false);
        }
    }
}
#endif

#endif // WAKE_COFFEE_H