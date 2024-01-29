#include <Arduino.h>
#include "config.h"
#include "elapsedMillis.h"

#include "main_waker.h"

#include "config_user.h" 
#include <SPIFFS.h>

#if ENABLE_WIFI
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
AsyncWebServer server_select(80);
DNSServer dns;

#if ENABLE_SERVER
#include "ui_server.h"
#endif
#endif

#if ENABLE_DISPLAY
#include "display.h"
#endif

#if ENABLE_ESPNOW
#include "util/espnow.h"
void ESPNOW_receiveBytes(uint8_t *data, uint8_t len); 
#endif

#if ENABLE_UI
#include "ui.h"
#endif

// ================================================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

#if ENABLE_DISPLAY
    display_setup();
#endif

#if ENABLE_WIFI 
    AsyncWiFiManager wifiManager(&server_select,&dns);
    wifiManager.setTimeout(180);
    
    if (!wifiManager.autoConnect("Poly-Waker - select WIFI here!"))
    {
        Serial.println("failed to connect and hit timeout");
        ESP.restart();
    }
    // wifiManager.setDebugOutput(true);
    // wifiManager.startConfigPortal("Poly-Waker - select WIFI here!");
    Serial.println(WiFi.macAddress());
#endif

#if ENABLE_ESPNOW
    ESPNOW_Init(ESPNOW_receiveBytes);
#endif

#if ENABLE_UI
    ui.setup();
#endif

    main_waker_setup();

    if (!SPIFFS.begin())
        Serial.println("SPIFFS could not initialize");
    config_setup();

#if ENABLE_SERVER
    server_setup();
    display_webserver_message();
    delay(2000);
#endif

    waker.setAlarmFromConfig();
    
#if ENABLE_AUTO_START
    waker.setAlarmRelativeIn(10);
#endif

    Serial.println("Starting loop() ...");
}
// ================================================================================================

int measure_us()
{
    static elapsedMicros time_elapsed = 0;
    int t = time_elapsed;
    time_elapsed = 0;
    return t;
}

int us_server, us_ui, us_waker, us_display, us_audio = 0;

void loop()
{
    measure_us();
    
#if ENABLE_SERVER
    server_loop();
    us_server += measure_us();
#endif

#if ENABLE_UI
    ui.loop();
    us_ui += measure_us();
#endif

    main_waker_loop();
    us_waker += measure_us();

#if ENABLE_DISPLAY
    display_loop();
    us_display += measure_us();
#endif

#if ENABLE_WAKE_BACKUP
    wake_backup_audio_loop();
    wake_backup_setActive(waker.isRinging() && (waker.since_alarm_started > (config.backup_start * 1000)));
    us_audio += measure_us();
#endif

#if ENABLE_DEBUG_PRINT_TASK_TIMINGS

    static elapsedMillis since_print = 0;
    static int count_since_print = 0;

    count_since_print++;

    if (since_print > 500)
    {
        since_print = 0;

        us_server /= count_since_print;
        us_ui /= count_since_print;
        us_display /= count_since_print;
        us_audio /= count_since_print;
        us_device /= count_since_print;

        int total = us_server + us_ui + us_display + us_audio + us_device;
        Serial.printf("TASKS: %6d = s %6d  u %6d  d %6d  a %6d dev %6d\n", total, us_server, us_ui, us_display, us_audio, us_device);
        
        count_since_print = 0;
    }
#endif
}

#if ENABLE_ESPNOW
// ########################## RX callback ##########################
void ESPNOW_receiveBytes(uint8_t *data, uint8_t len)
{
    static bool light_on = false;
    memcpy(&msg, data, len);
    Serial.printf("ESPNOW_receiveBytes: %d - %f\n", msg.index, msg.value);

    light_on = !light_on;
    digitalWrite(LED_BUILTIN, light_on);
}
#endif
