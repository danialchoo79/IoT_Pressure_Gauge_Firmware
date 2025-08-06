#include "RTC.h"

// NTP Server Settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600; // GMT+8 for Singapore
const int daylightOffset_sec = 0;    // Adjust if DST is used 

void GetCurrentTime()
{
    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

    struct tm timeinfo;

    if(getLocalTime(&timeinfo)) {
        Serial.println(&timeinfo, "Now is: %A, %B %d %Y %H:%M:%S");
    } else {
        Serial.println("Failed to obtain time");
    }
}