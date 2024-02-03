#pragma once
#include "wake_method.h"

#include "util/espnow.h"

class WakeCoffee : public WakeMethod
{
public:
    void control(float value) override
    {
        Serial.printf("WakeLight::control( %f ) \n", value);
        // TODO Check for big enough change ?!
        _current = constrain(value, 0.0, 1.0);

        set_OK_to_use();

        send_cmd(0, !_current);
    }

    void set(float value) override
    {
        if (_since_last_msg < 60)
            return;

        value = constrain(value, 0.0, 1.0);

        static int num_msg = 0;
        if (_current != value)
            num_msg = 0;

        if (num_msg < 3)
        {
            num_msg++;
            control(value); // send 3x control per 1x set

            if (value > 0.0)
                _since_last_active = 0;
        }
    }

    void loop()
    {
        if (_current > 0)
        {
            // AUTO TURN OFF after 8 minutes
            if (_since_last_active > (8 * MIN_TO_MS))
                set(0);
        }
    }

    void reset()
    {
        control(true);
        control(false);
        set_OK_to_use();
    }

#define COOLDOWN_TIME_MS 60 * MIN_TO_MS
    bool is_OK_to_use() { return _since_last_active > COOLDOWN_TIME_MS; }
    void set_OK_to_use() { _since_last_active = COOLDOWN_TIME_MS; }
private:
    elapsedMillis _since_last_msg = 0;
    elapsedMillis _since_last_active = 5 * 60 * MIN_TO_MS;

    void send_cmd(int index, float value) // 0.0 - 1.0
    {
        // Serial.println("send_cmd(" + String(msg.value) + ")");
        ESPNOW_send_cmd(MAC_ADDRESS_COFFEE, index, value); // TODO dynamic address
        _since_last_msg = 0;
        delayMicroseconds(1000); // TODO is this really necessary ?!?
    }
};
