#include <Arduino.h>
#include "config.h"
#include "elapsedMillis.h"

#if ENABLE_WIFI
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>         //https://github.com/tzapu/WiFiManager

AsyncWebServer server_select(80);
DNSServer dns;
#endif

#include "main_waker.h"

#if ENABLE_DISPLAY
#include "display.h"
#endif


#if ENABLE_WAKE_BACKUP
#include "wake_backup.h"
#endif

#if ENABLE_ESPNOW
#include "util/espnow.h"
void ESPNOW_receiveBytes(uint8_t *data, uint8_t len);
#endif

#if ENABLE_WAKE_LIGHT
#include "wake_light.h"
#endif

#if ENABLE_WAKE_COFFEE
#include "wake_coffee.h"
#endif

#if ENABLE_UI
#include "ui.h"
extern UI ui;
void arcadeButtonLongPress();
void arcadeButtonClicked();
void rotaryButtonClicked();
#endif

#include <SPIFFS.h>

#if ENABLE_SERVER
#include "ui_server.h"
#endif

void srv_setup();

// ================================================================================================
void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);

#if ENABLE_DISPLAY
    display_setup();

    display.setTextSize(1);
    display.setCursor(0, 2);
    display.printf("checking");
    display.setTextSize(2);
    display.setCursor(0, 17);
    display.printf("WiFi...");
    display.display();
#endif

#if ENABLE_WIFI 
    AsyncWiFiManager wifiManager(&server_select,&dns);
#if 1
    if (!wifiManager.autoConnect("Poly-Waker - select WIFI here!"))
    {
        Serial.println("failed to connect and hit timeout");
        ESP.restart();
    }
    #else
    wifiManager.setDebugOutput(true);
    wifiManager.startConfigPortal("Poly-Waker - select WIFI here!");    
    #endif
    Serial.println(WiFi.macAddress());
#endif

#if ENABLE_ESPNOW
    ESPNOW_Init(ESPNOW_receiveBytes);
#endif

#if ENABLE_WAKE_BACKUP
    wake_backup_setup();
#endif

    actual_time_setup();
    main_waker_setup();
    setMode(IDLE);

#if ENABLE_UI
    ui.setup();
    ui.arcadeButton.attachClick(arcadeButtonClicked);
    ui.rotaryButton.attachClick(rotaryButtonClicked);

    ui.arcadeButton.attachLongPressStart(arcadeButtonLongPress);
#endif

    if (!SPIFFS.begin())
        Serial.println("SPIFFS could not initialize");
        
    config_setup();

#if ENABLE_SERVER
    server_setup();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 5);
    display.printf("Find me:");
    display.setCursor(0, 20);
    display.printf("http://waker.local");
    display.display();
    delay(2000);
#endif

    waker.retrieveAlarmFromConfig();
    
#if ENABLE_AUTO_START
    waker.setAlarmRelativeIn(10);
#endif

    Serial.println("Starting loop() ...");
}

void main_waker_loop();

// ================================================================================================
elapsedMicros time_elapsed = 0;
elapsedMillis since_print = 0;
int count_since_print = 0;

int getMicro()
{
    int t = time_elapsed;
    time_elapsed = 0;
    return t;
}

int us_server, us_ui, us_display, us_audio, us_device;
void loop()
{
    getMicro();
    
#if ENABLE_SERVER
    server_loop();
    us_server += getMicro();
#endif

#if ENABLE_UI
    ui.loop();
    us_ui += getMicro();
#endif

#if ENABLE_DISPLAY
    display_loop();
    us_display += getMicro();
#endif

#if ENABLE_WAKE_BACKUP
    wake_backup_audio_loop();
    wake_backup_setActive(waker.isRinging && (waker.since_alarm_started > (config.backup_start * 1000)));
    us_audio += getMicro();
#endif

    main_waker_loop();
    us_device += getMicro();

#if ENABLE_DEBUG_PRINT_TASK_TIMINGS
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
bool light_on = false;
// ########################## RX callback ##########################
void ESPNOW_receiveBytes(uint8_t *data, uint8_t len)
{
    memcpy(&msg, data, len);
    //    Serial.printf("ESPNOW_receiveBytes: %d - %f\n",  msg.index, msg.value);

    light_on = !light_on;
    digitalWrite(LED_BUILTIN, light_on);
}
#endif
