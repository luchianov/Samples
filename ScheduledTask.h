/**
 * @file ScheduledTask.h
 * @author Slava Luchianov
 * @brief
 * @version 0.1
 * @date 2023-12-08
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <memory>

#include "StreamLogger.h"

class ScheduledTask
{
public:
    ScheduledTask(const std::string &schedule, const std::string &config = "");
    bool shouldRunNow();
    std::string getSchedule() const;
    std::string getConfig() const;

private:
    std::vector<int> minutes, hours, daysOfMonth, months, daysOfWeek;
    std::string extraConfig; // This holds the extra configuration, like the JSON command
    std::string origSchedule;
    std::string lastExecutionID;

    void parseSchedule(const std::string &schedule);
    std::vector<int> parseField(const std::string &field);
    bool matches(int timeValue, const std::vector<int> &values);
    std::string createExecutionID(const std::tm &time);
};
