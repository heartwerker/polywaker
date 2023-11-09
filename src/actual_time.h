#pragma once
#ifndef ACTUAL_TIME_H
#define ACTUAL_TIME_H

#include "time.h"

#define USE_WITH_TIME_ZONE 1

#if USE_WITH_TIME_ZONE
#define TIME_ZONE_STRING "CET-1CEST,M3.5.0,M10.5.0/3" // = berlin tz:
#else
const long gmtOffset_sec = 1 * 3600;
const int daylightOffset_sec = 3600;
#endif

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const char *ntpServer3 = "time.google.com";

void printLocalTime();

void actual_time_setup()
{
#if USE_WITH_TIME_ZONE
  configTzTime(TIME_ZONE_STRING, ntpServer1, ntpServer2, ntpServer3);
#else
// configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
#endif
  printLocalTime();
}

struct tm actual_time_get()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
  }
  return timeinfo;
}

void printLocalTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay, 10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}

#endif // ACTUAL_TIME_H