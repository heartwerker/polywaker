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

bool parse(StaticJsonDocument<200> *pDoc, Config::Parameter *parameter)
{
  const String type = static_cast<const char *>((*pDoc)["type"]);
  const int value = (*pDoc)["value"];

  if (type == parameter->name)
  {
    parameter->value = int(value);
    config.save();
    sendJson(parameter->name, String(value));
    return true;
  }
  return false;
}

void send_wake_control()
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
    Serial.println("Client Nr. " + String(num) + " disconnected");
    break;
  }
  case WStype_CONNECTED:
  {
    Serial.println("Client Nr. " + String(num) + " connected");

    // send variables to connected web client -> as optimization: could send it just to the new client "num" in future

    for (auto param : config.parameters)
      sendJson(*param);

    send_wake_control();
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
      const String type = static_cast<const char *>(doc["type"]);
      const int value = doc["value"];

      printf("Command: %s\nValue: %d\n", type, value);

      bool redraw = false;
      redraw |= parse(&doc, &config.light_start);
      redraw |= parse(&doc, &config.light_end);
      redraw |= parse(&doc, &config.music_start);
      redraw |= parse(&doc, &config.music_end);
#if ENABLE_WAKE_COFFEE
      redraw |= parse(&doc, &config.coffee_start);
#endif
      redraw |= parse(&doc, &config.backup_start);

      if (redraw)
        updateGraphs();

      parse(&doc, &config.snooze_time);

      bool alarmChanged = false;
      alarmChanged |= parse(&doc, &config.alarm_enabled);
      alarmChanged |= parse(&doc, &config.alarm_hour);
      alarmChanged |= parse(&doc, &config.alarm_minute);
      
      if (alarmChanged)
        waker.setAlarmFromConfig();

      if (type.startsWith("control_"))
      {
        Serial.printf("Received %s ( %d ) \n", type.c_str(), value);

        if (type == "control_light")
          controlLight(float(value) / 100.0);

        if (type == "control_music")
          controlMusic(float(value) / 100.0);

      #if ENABLE_WAKE_COFFEE
        if (type == "control_coffee")
          controlCoffee(bool(value));
      #endif
          send_wake_control();
      }
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
      send_wake_control();
  }
#endif
}

#endif