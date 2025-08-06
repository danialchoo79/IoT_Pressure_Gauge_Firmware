#ifndef RTC_H
#define RTC_H

#include "time.h"
#include <Arduino.h>

// NTP Server Settings
extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;

void GetCurrentTime();

#endif