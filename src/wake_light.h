#pragma once
#include "wake_method.h"

#include "util/espnow.h"

class WakeLight : public WakeMethod
{
public:
    void control(float value) override
    {
        // todo should this check here only be in set() ?
        if (fabs(_current - value) > 0.002 
        || (value == 0.0 && _current != 0))
        {
            _current = constrain(value, 0.0, 1.0);

#if USER_IS_LEO
            send_cmd(0, mapConstrainf(_current, 0.0, 0.8, 0, 1));
            send_cmd(1, mapConstrainf(_current, 0.2, 1.0, 0, 1));
            for (int l = 2; l < 4; l++)
            {
                send_cmd(l, _current);
                delayMicroseconds(1000);
            }
#elif USER_IS_JANEK
            for (int l = 0; l < 4; l++)
            {
                send_cmd(l, _current);
                delayMicroseconds(1000);
            }
#elif USER_IS_DAVE
            // map _current to W, R, G channels)
            send_cmd(0, (_current - 0.2) / 0.8);          // white
            send_cmd(1, _current * 5);                    // red
            send_cmd(2, constrain(_current * 2, 0, 0.7)); // green

            if (_since_is_part_of_ritual < 200)
            {
                constexpr float offset = 0.5;
                if (_current >= offset)
                    send_cmd(21, mapConstrainf(_current, offset, 1.0, 0.0, 1.0));
            }
#endif
        }
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
        }
    }

    // little hack to also trigger motor
    void setRitualStep() { _since_is_part_of_ritual = 0; }

private:
    elapsedMillis _since_last_msg = 0;
    elapsedMillis _since_is_part_of_ritual = 0;

    void send_cmd(int index, float value) // 0.0 - 1.0
    {
        // Serial.println("send_cmd(" + String(msg.value) + ")");
        ESPNOW_send_cmd(MAC_ADDRESS_LIGHT, index, value); // TODO dynamic address
        _since_last_msg = 0;
        delayMicroseconds(1000); // TODO is this really necessary ?!?
    }
};
