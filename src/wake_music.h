#pragma once
#include "wake_method.h"

#include "util/http.h"

class WakeMusic : public WakeMethod
{
public:
    void control(float value) override
    {
        // todo should this check here only be in set() ?
        if (fabs(_current - value) > 0.01 
        || (value == 0.0 && _current != 0))
        {
            Serial.printf("WakeMusic::control( %f ) \n", value);
            _current = constrain(value, 0.0f, 1.0f);

            // remap to 0.1 - 0.6
            float vol_adj = mapConstrainf(_current, 0.0f, 1.0f, 0.1f, 0.6f);

            static int last_vol_i = -1;
            int vol_i = int(vol_adj * 100.0f);
            if (vol_i != last_vol_i)
            {
                last_vol_i = vol_i;
                send_http("/volume?volume=" + String(vol_i));
            }

            changeRadio(_current > 0.0);

        }
    }

    void set(float value) override
    {
        if (_current < 1.0 && value >= 1.0) // when value becomes full volume 1.0
        {
            send_http("/random_message");
            Serial.println("Triggered /random_message !");
        }
        control(value);
    }

    // ========================================================
    void changeRadio(bool play)
    {
        static bool _play = false;
        if (_play != play)
        {
            _play = play;
            send_http(_play ? "/radioplay" : "/radiostop");
        }
    }
};
