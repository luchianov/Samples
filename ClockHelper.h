#pragma once
#include <Arduino.h>
#include <ctime>
#include <sys/time.h>
#include <iomanip>
#include <sstream>

#include <Wire.h>
#include <RTClib.h>

#include "StreamLogger.h"

class ClockHelper : RTC_DS3231
{
public:
    ClockHelper(){
        // this->setup();
    };

    bool delayed_setup();
    void time_stamp_to_serial();
    bool set_controller_clock(const std::string &dateTimeString);
    bool synchronize_esp32_to_rtc();
    bool synchronize_esp32_to_rtc_at_24_hours();
    bool set_esp32_clock(int year, int month, int day, int hour, int minute, int second);
};

// Global logger instance declaration
extern ClockHelper runtime_clock_helper;