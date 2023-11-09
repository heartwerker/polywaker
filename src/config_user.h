#ifndef CONFIG_USER_H
#define CONFIG_USER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

void config_setup();

// ==============================

class Config
{
public:
    // ==============================
    // Defaults configured here:

    // all _time in minutes
    int coffee_start = 0;
    int light_start = 0;
    int light_end = 5;
    int music_start = 0;
    int music_end = 15;
    int backup_start = 29;

    int snooze_time = 5;

    int alarm_enabled = true;
    int alarm_hour = 9;
    int alarm_minute = 0;

    // ==============================
    // constants / not changeable
    int display_sleep_time_s = 60 * 3;
    int setting_inactivity_s = 15;
    int backup_fade_relative_s = 120;
    // ==============================

    Config(){}

    bool save() const
    {
        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            Serial.println("Failed to open config file for writing");
            return false;
        }

        DynamicJsonDocument doc(1024);
        doc["light_start"] = light_start;
        doc["light_end"] = light_end;
        doc["music_start"] = music_start;
        doc["music_end"] = music_end;
        doc["coffee_start"] = coffee_start;
        doc["backup_start"] = backup_start;
        doc["snooze_time"] = snooze_time;

        doc["alarm_enabled"] = alarm_enabled;
        doc["alarm_hour"] = alarm_hour;
        doc["alarm_minute"] = alarm_minute;

        if (serializeJson(doc, configFile) == 0)
        {
            Serial.println("Failed to write config file");
            configFile.close();
            return false;
        }

        configFile.close();
        return true;
    }

    bool load()
    {
        File configFile = SPIFFS.open("/config.json", "r");
        if (!configFile)
        {
            Serial.println("Failed to open config file");
            return false;
        }

        size_t size = configFile.size();
        if (size > 1024)
        {
            Serial.println("Config file size is too large");
            configFile.close();
            return false;
        }

        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        configFile.close();

        DynamicJsonDocument doc(1024);
        auto error = deserializeJson(doc, buf.get());
        if (error)
        {
            Serial.println("Failed to parse config file");
            return false;
        }

        light_start = doc["light_start"];
        light_end = doc["light_end"];
        music_start = doc["music_start"];
        music_end = doc["music_end"];
        coffee_start = doc["coffee_start"];
        backup_start = doc["backup_start"];
        snooze_time = doc["snooze_time"];

        alarm_enabled = doc["alarm_enabled"];
        alarm_hour = doc["alarm_hour"];
        alarm_minute = doc["alarm_minute"];

        return true;
    }
};

// ==============================
// Global instance
Config config;

void config_setup()
{
    config.load();
    
    Serial.println("Loaded user settings:");
    Serial.printf("light_start: %d\n", config.light_start);
    Serial.printf("light_end: %d\n", config.light_end);
    Serial.printf("music_start: %d\n", config.music_start);
    Serial.printf("music_end: %d\n", config.music_end);
    Serial.printf("coffee_start: %d\n", config.coffee_start);
    Serial.printf("backup_start: %d\n", config.backup_start);
    Serial.printf("snooze_time: %d\n", config.snooze_time);

    Serial.printf("alarm_enabled: %d\n", config.alarm_enabled);
    Serial.printf("alarm_hour: %d\n", config.alarm_hour);
    Serial.printf("alarm_minute: %d\n", config.alarm_minute);
}

#endif // CONFIG_USER_H
