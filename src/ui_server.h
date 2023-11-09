#ifndef SERVER_H
#define SERVER_H

// ---------------------------------------------------------------------------------------
// Server utilizes WiFi, WebSockets, and JSON for configuration and control.
//
// Requires SPIFFS for file system operations. For setup, refer to:
// https://github.com/me-no-dev/arduino-esp32fs-plugin/releases/
// https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
//
// Icon from: https://icons8.com/icons/set/favicon
// Based on code by mo thunderz, updated last on 11.09.2022.
// https://github.com/mo-thunderz/Esp32WifiPart4
// ---------------------------------------------------------------------------------------

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include "elapsedMillis.h"
#include "ui_server_utils.h"
#include "wake_light.h"
#include "wake_coffee.h"
#include "config.h"

// Server configuration
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
const int FADE_LENGTH = 30;

// Graph arrays for wake-up components
float graph_light[FADE_LENGTH];
float graph_music[FADE_LENGTH];
float graph_backup[FADE_LENGTH];
#if ENABLE_WAKE_COFFEE
float graph_coffee[FADE_LENGTH];
#endif

void computeGraph_linearFade(int delay_time, int fade_time, float *values)
{
  for (int i = 0; i < FADE_LENGTH; i++)
  {
    if (i <= delay_time)
      values[i] = 0;
    else if (i <= (delay_time + fade_time))
      values[i] = float(i - delay_time) / float(fade_time);
    else
      values[i] = 1.0;

    values[i] *= 100.0;
  }
}

void computeGraph_step(int delay_time, float *values)
{
  for (int i = 0; i < FADE_LENGTH; i++)
  {
    if (i < delay_time)
      values[i] = 0;
    else
      values[i] = 1.0;

    values[i] *= 100.0;
  }
}

void updateGraphs()
{
  computeGraph_linearFade(config.light_start, config.light_end, graph_light);
  sendJsonArray("graph_light", graph_light, FADE_LENGTH);

  computeGraph_linearFade(config.music_start, config.music_end, graph_music);
  sendJsonArray("graph_music", graph_music, FADE_LENGTH);

  computeGraph_step(config.backup_start, graph_backup);
  sendJsonArray("graph_backup", graph_backup, FADE_LENGTH);

#if ENABLE_WAKE_COFFEE
  computeGraph_step(config.coffee_start, graph_coffee);
  sendJsonArray("graph_coffee", graph_coffee, FADE_LENGTH);
#endif

  Serial.println("Updated graphs");
}

bool handleString(String type, int value, String config_name, int *config_value)
{
  if (type == config_name)
  {
    *config_value = int(value);
    config.save();
    sendJson(config_name, String(value));
    return true;
  }
  return false;
}

void send_wake_control_values()
{
  sendJson("control_light", String(control_light * 100));
  sendJson("control_music", String(control_music * 100));
#if ENABLE_WAKE_COFFEE
  sendJson("control_coffee", String(control_coffee));
#endif
}

// the parameters of this callback function are always the same -> num: id of the client who send the event, ws_type: ws_type of message, payload: actual data sent and length: length of payload
void webSocketEvent(byte num, WStype_t ws_type, uint8_t *payload, size_t length)
{
  switch (ws_type)
  {                        
  case WStype_DISCONNECTED: 
  {
    Serial.println("Client " + String(num) + " disconnected");
    break;
  }
  case WStype_CONNECTED:
  {
    Serial.println("Client " + String(num) + " connected");

    // send variables to newly connected web client -> as optimization step one could send it just to the new client "num", but for simplicity I left that out here
    sendJson("alarm_enabled", String(config.alarm_enabled));
    sendJson("alarm_hour", String(config.alarm_hour));
    sendJson("alarm_minute", String(config.alarm_minute));

    sendJson("light_start", String(config.light_start));
    sendJson("light_end", String(config.light_end));
    sendJson("music_start", String(config.music_start));
    sendJson("music_end", String(config.music_end));
#if ENABLE_WAKE_COFFEE
    sendJson("coffee_start", String(config.coffee_start));
#endif
    sendJson("backup_start", String(config.backup_start));
    sendJson("snooze_time", String(config.snooze_time));

    send_wake_control_values();
    updateGraphs();
    Serial.println("Sent initial values");

    break;
  }
  case WStype_TEXT:
  { 
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }
    else
    {
      // JSON string was received correctly, so information can be retrieved:
      const char *l_type = doc["type"];
      const int l_value = doc["value"];
      const String type = String(l_type);

      Serial.println("Command: " + type);
      Serial.println("Value: " + String(l_value));

      bool graphChanged = false;
      graphChanged |= handleString(type, l_value, "light_start", &config.light_start);
      graphChanged |= handleString(type, l_value, "light_end", &config.light_end);
      graphChanged |= handleString(type, l_value, "music_start", &config.music_start);
      graphChanged |= handleString(type, l_value, "music_end", &config.music_end);
#if ENABLE_WAKE_COFFEE
      graphChanged |= handleString(type, l_value, "coffee_start", &config.coffee_start);
#endif
      graphChanged |= handleString(type, l_value, "backup_start", &config.backup_start);

      if (graphChanged)
        updateGraphs();

      handleString(type, l_value, "snooze_time", &config.snooze_time);

      bool alarmChanged = false;
      alarmChanged |= handleString(type, l_value, "alarm_enabled", &config.alarm_enabled);
      alarmChanged |= handleString(type, l_value, "alarm_hour", &config.alarm_hour);
      alarmChanged |= handleString(type, l_value, "alarm_minute", &config.alarm_minute);
      if (alarmChanged)
        waker.retrieveAlarmFromConfig();

      if (type == "control_light")
      {
        float val = float(int(l_value)) / 100.0;
        Serial.println("Received control_light to " + String(int(l_value) + " (= " + String(val) + ")"));
        controlLight(val);
        send_wake_control_values();
      }

      if (type == "control_music")
      {
        float val = float(int(l_value)) / 100.0;
        Serial.println("Received control_music to " + String(int(l_value) + " (= " + String(val) + ")"));
        controlMusic(val);
        send_wake_control_values();
      }

#if ENABLE_WAKE_COFFEE
      if (type == "control_coffee")
      {
        Serial.println("Received control_coffee to " + String(int(l_value)));
        controlCoffee(bool(int(l_value)));
        send_wake_control_values();
      }
#endif
    }

    break;
  }
  }
}

void server_setup()
{

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "File not found");
  });

  server.serveStatic("/", SPIFFS, "/");

  webSocket.begin();                 // start websocket
  webSocket.onEvent(webSocketEvent); // define a callback function -> what does the ESP32 need to do when an event from the websocket is received? -> run function "webSocketEvent()"

  if (!MDNS.begin("waker")) // sets hostname to waker.local
    Serial.println("Error setting up mDNS responder!");

  Serial.println("mDNS responder started");

  server.begin(); // start server after the websocket

  Serial.printf("Find Server here: http://%s\n\n", WiFi.localIP().toString().c_str());
}

void server_loop()
{
  webSocket.loop();

// TODO: mechanism to only send when control values changed
#if 1 // Send wake control values every second
  static elapsedMillis timeElapsed;
  if (timeElapsed > 1000)
  {
    timeElapsed = 0;
    if (webSocket.connectedClients() > 0)
      send_wake_control_values();
  }
#endif
}

#endif