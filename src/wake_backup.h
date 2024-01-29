#pragma once
#ifndef wake_backup_H
#define wake_backup_H

// TODO REWORK THIS FILE: generate a proper beep sound non-blocking
// TODO:  (also rename to wake_audio or so)

#include <Arduino.h>
#include "config.h"

#include <driver/i2s.h>
#include <elapsedMillis.h>

float alarm_volume = 1;
float alarm_period_ms = 1000;

void wake_backup_setVolume(float volume)
{
    alarm_volume = constrain(volume, 0.0, 1.0);
}

void wake_backup_setFrequency(float hz)
{
    alarm_period_ms = 1000.0 / hz;
    // Serial.println(alarm_period_ms);
}

#define SAMPLE_RATE (44100)

void wake_backup_setup()
{
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER + I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64};

    i2s_pin_config_t pin_config = {
        .bck_io_num = PIN_I2S_BCK,
        .ws_io_num = PIN_I2S_LRCLK,
        .data_out_num = PIN_I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE};

    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

double phase = 0;
double amplitude = 0;

elapsedMillis sinceBeepOn = 0;
elapsedMillis since_alarm_render = 0;

bool active = false;
void wake_backup_setActive(bool value)
{
    if (active == value)
        return;

    active = value;

    if (!active)
        i2s_zero_dma_buffer(I2S_NUM_0); // mute output

    if (active)
        wake_backup_setVolume(0.0);
}

#define RENDER_MILLISECONDS 100
void wake_backup_audio_loop()
{
    if (active)
    {
        if (since_alarm_render >= RENDER_MILLISECONDS)
        {
            since_alarm_render -= RENDER_MILLISECONDS;

            if (sinceBeepOn > alarm_period_ms)
            {
                amplitude = 0;
                sinceBeepOn = 0;
            }

            if ((sinceBeepOn < (alarm_period_ms / 2)) && active)
            {
                int16_t sample;
                double freq = 440; // 880.0;
                size_t bytes_written;

                // #define FADE_DURATION (SAMPLE_RATE / 10)  // 0.1 seconds for faster fade-in/out
                int fade_durtion_ms = alarm_period_ms / 3;

                float step = 1000.0 / float(SAMPLE_RATE) / float(fade_durtion_ms);

                for (int i = 0; i < (float(SAMPLE_RATE) / (1000.0 / RENDER_MILLISECONDS)); ++i)
                {
                    if (amplitude < 1.0)
                        amplitude += step;

#if 0
            // Fade-out effect
            if (i > SAMPLE_RATE / 2 - fade_durtion_ms)
            {
                amplitude = (double)(SAMPLE_RATE / 2 - i) / fade_durtion_ms;
            }
#endif

                    // simple converstion of linear amplitude 0-1 to audible scale
                    // amplitude = pow(amplitude, 0.5);

                    // Convert amplitude in range 0.0 to 1.0 to int16 range and apply to sine wave

                    sample = 32767.0 * amplitude * alarm_volume * sin(phase);

                    phase += 2.0 * PI * freq / SAMPLE_RATE;
                    if (phase >= 2.0 * PI)
                        phase -= 2.0 * PI;

                    // Send samples to I2S
                    const int16_t samples[2] = {sample, sample};
                    i2s_write(I2S_NUM_0, (const char *)samples, sizeof(samples), &bytes_written, 100);
                }
            }
            else
            {
                i2s_zero_dma_buffer(I2S_NUM_0); // mute output
            }
        }
    }
}

#endif // wake_backup_H
