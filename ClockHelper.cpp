#include "ClockHelper.h"

/**
 * @brief Initializes the RTC and sets its time if it lost power.
 * @return True if initialization is successful, false if RTC is not found or another error occurs.
 *
 * This function attempts to begin communication with the RTC. If the RTC had lost power,
 * it sets the RTC time to the time when the sketch was compiled. It also turns off the
 * square wave output to avoid noise.
 */
bool ClockHelper::delayed_setup()
{
    if (!this->begin())
    {
        stream_logger.println("Couldn't find RTC");
        return false;
    }

    if (this->lostPower())
    {
        stream_logger.println("RTC lost power, setting the time!");
        // Following line sets the RTC to the date & time this sketch was compiled
        this->adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    // Turning off the output of the calibrated frequency, cause we don't want the noise
    this->writeSqwPinMode(DS3231_OFF);

    this->synchronize_esp32_to_rtc();

    return true;
}

/**
 * @brief Outputs the current time stamp to the serial stream.
 *
 * Retrieves the current system time and formats it into a human-readable string,
 * which is then output to the serial stream. The format used is 'YYYY-MM-DD HH:MM:SS'.
 */
void ClockHelper::time_stamp_to_serial()
{
    char buffer[80];
    std::time_t t = std::time(nullptr);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
    stream_logger.printf("========  %s  ========\n", buffer);
}

/**
 * @brief Sets the controller clock to a specified date and time.
 * @param dateTimeString The date and time in ISO 8601 format (YYYY-MM-DDTHH:MM:SS).
 * @return True if the time was set successfully, false otherwise.
 *
 * This function parses the given date and time string, adjusts the RTC,
 * and synchronizes the ESP32 internal clock to the RTC.
 */
bool ClockHelper::set_controller_clock(const std::string &dateTimeString)
{
    stream_logger.printf("ClockHelper::set_controller_clock(%s)\n", dateTimeString.c_str());

    std::tm tm = {};
    std::istringstream time_stream(dateTimeString);
    time_stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S"); // expects YYYY-MM-DDTHH:MM:SS
    if (time_stream.fail())
    {
        stream_logger.println("Error: Invalid date/time string.");
        return false;
    }

    // set the RTC with an explicit date & time
    this->adjust(DateTime(tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec));

    return this->synchronize_esp32_to_rtc();
}

/**
 * @brief Synchronizes the ESP32 internal clock to the RTC time.
 * @return True if synchronization is successful, false otherwise.
 *
 * Retrieves the current time from the RTC and updates the ESP32 internal clock.
 */
bool ClockHelper::synchronize_esp32_to_rtc()
{
    // Get the time from RTC
    DateTime now = this->now();

    // Set ESP32 internal clock
    return set_esp32_clock(
        now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());
}

/**
 * @brief Synchronizes the ESP32 clock to RTC every 24 hours.
 * @return True if synchronization occurs, false if not yet 24 hours since last sync.
 *
 * Checks if 24 hours have passed since the last synchronization and,
 * if so, synchronizes the ESP32 clock to the RTC.
 */
bool ClockHelper::synchronize_esp32_to_rtc_at_24_hours()
{
    // Every 24 hours, synchronize time
    static unsigned long lastSyncTime = 0;
    if (millis() - lastSyncTime > 86400000)
    { // 86400000 ms = 24 hours
        lastSyncTime = millis();
        return synchronize_esp32_to_rtc();
    }
    return false;
}

/**
 * @brief Sets the ESP32 system clock.
 * @param year Year to set (Note: the year should be in full format, e.g., 2023).
 * @param month Month to set (1-12).
 * @param day Day to set (1-31).
 * @param hour Hour to set (0-23).
 * @param minute Minute to set (0-59).
 * @param second Second to set (0-59).
 * @return True if the time was set successfully, false otherwise.
 *
 * Converts the provided date and time into a format suitable for the ESP32 system clock
 * and sets the system time accordingly. The function adjusts the year to be compatible
 * with the std::tm structure by subtracting 2000 from the input year.
 */
bool ClockHelper::set_esp32_clock(int year, int month, int day, int hour, int minute, int second)
{
    std::tm tm = {
        // The order is important to keep compiler happy
        .tm_sec = second,
        .tm_min = minute,
        .tm_hour = hour,
        .tm_mday = day,
        .tm_mon = month,
        .tm_year = year - 2000, // My version of ESP32 libraries counts the year starting from 2000
    };

    // Convert tm to time_t then to timeval
    std::time_t t = std::mktime(&tm);
    if (t == -1)
    {
        stream_logger.println("Error: Unable to make time.");
        return false;
    }

    // Now we need to create a timeval struct and set it using settimeofday
    struct timeval now = {.tv_sec = t, .tv_usec = 0};
    if (settimeofday(&now, nullptr) != 0)
    {
        stream_logger.println("Error: Failed to set time.");
        return false;
    }

    // For logging, convert the time to a string
    char buf[64];
    strftime(buf, sizeof(buf), "%c", &tm);
    stream_logger.printf("Setting controller clock to: %s\n", buf);
    return true;
}
