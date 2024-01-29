#pragma once
#ifndef ACTUAL_TIME_H
#define ACTUAL_TIME_H

using Time = struct tm; 

#include "time.h" // ESP library

#define USE_WITH_TIME_ZONE 1

#if USE_WITH_TIME_ZONE
#define TIME_ZONE_STRING "CET-1CEST,M3.5.0,M10.5.0/3" // = Berlin
#else
const long gmtOffset_sec = 1 * 3600;
const int daylightOffset_sec = 3600;
#endif

const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";
const char *ntpServer3 = "time.google.com";

void actual_time_setup();
Time actual_time_get();
void printTime(Time time);

void actual_time_setup()
{
#if USE_WITH_TIME_ZONE
  configTzTime(TIME_ZONE_STRING, ntpServer1, ntpServer2, ntpServer3);
#else
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
#endif
  printTime(actual_time_get());
}

Time actual_time_get() 
{
  Time now; 
  if (!getLocalTime(&now))
    Serial.println("Failed to obtain time");
    
  return now;
}

void printTime(Time time)
{
  Serial.println(&time, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&time, "%A");
  Serial.print("Month: ");
  Serial.println(&time, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&time, "%d");
  Serial.print("Year: ");
  Serial.println(&time, "%Y");
  Serial.print("Hour: ");
  Serial.println(&time, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&time, "%I");
  Serial.print("Minute: ");
  Serial.println(&time, "%M");
  Serial.print("Second: ");
  Serial.println(&time, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour, 3, "%H", &time);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay, 10, "%A", &time);
  Serial.println(timeWeekDay);
  Serial.println();
}

#endif // ACTUAL_TIME_H