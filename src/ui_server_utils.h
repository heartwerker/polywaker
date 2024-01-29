#ifndef UI_SERVER_UTILS_H
#define UI_SERVER_UTILS_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

extern WebSocketsServer webSocket;

// TODO why "type" and not "name" ?
// Sends a JSON object to all connected WebSocket clients
void sendJson(const Config::Parameter &param)
{
    if (webSocket.connectedClients() > 0)
    { // Only send if there are connected clients
        StaticJsonDocument<200> doc;
        doc["type"] = param.name;
        doc["value"] = param.value;
        String jsonString;
        serializeJson(doc, jsonString);
        webSocket.broadcastTXT(jsonString);
        Serial.println("Sent JSON: " + jsonString); // Debug output
    }
}

// Sends a JSON object to all connected WebSocket clients
void sendJson(const String &type, const String &value)
{
    if (webSocket.connectedClients() > 0)
    { // Only send if there are connected clients
        StaticJsonDocument<200> doc;
        doc["type"] = type;
        doc["value"] = value;
        String jsonString;
        serializeJson(doc, jsonString);
        webSocket.broadcastTXT(jsonString);
        Serial.println("Sent JSON: " + jsonString); // Debug output
    }
}

const int MAX_ARRAY_LENGTH = 100; // Defines the maximum length for JSON arrays
// Sends a JSON array to all connected WebSocket clients
void sendJsonArray(const String &type, const float arrayValues[], int length)
{
    if (webSocket.connectedClients() > 0 && length > 0)
    { // Only send if there are connected clients and array has elements

        String jsonString = ""; 
        const size_t CAPACITY = JSON_ARRAY_SIZE(MAX_ARRAY_LENGTH) + 100;
        StaticJsonDocument<CAPACITY> doc;
        JsonArray valueArray = doc.createNestedArray("value");

        for (int i = 0; i < length; ++i)
            valueArray.add(arrayValues[i]);
        
        doc["type"] = type;
        serializeJson(doc, jsonString);
        webSocket.broadcastTXT(jsonString);
    }
}

#endif // UI_SERVER_UTILS_H
