/**
 * @file ScheduledTask.cpp
 * @author Slava Luchianov
 * @brief
 * @version 0.1
 * @date 2023-12-08
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "ScheduledTask.h"

ScheduledTask::ScheduledTask(const std::string &schedule, const std::string &config)
    : origSchedule(schedule), extraConfig(config)
{
    parseSchedule(schedule);
}

bool ScheduledTask::shouldRunNow()
{
    std::time_t now = std::time(nullptr);
    std::tm &ltm = *std::localtime(&now);

    if (matches(ltm.tm_min, minutes) &&
        matches(ltm.tm_hour, hours) &&
        matches(ltm.tm_mday, daysOfMonth) &&
        matches(ltm.tm_mon + 1, months) &&
        matches(ltm.tm_wday, daysOfWeek))
    {

        std::string currentExecutionID = createExecutionID(ltm);
        if (lastExecutionID != currentExecutionID)
        {
            lastExecutionID = currentExecutionID;
            return true;
        }
    }

    return false;
}

std::string ScheduledTask::getSchedule() const
{
    return origSchedule;
}

std::string ScheduledTask::getConfig() const
{
    return extraConfig;
}

void ScheduledTask::parseSchedule(const std::string &schedule)
{
    std::istringstream ss(schedule);
    std::string field;
    for (int i = 0; i < 5 && std::getline(ss, field, ' '); ++i)
    {
        switch (i)
        {
        case 0:
            minutes = parseField(field);
            break;
        case 1:
            hours = parseField(field);
            break;
        case 2:
            daysOfMonth = parseField(field);
            break;
        case 3:
            months = parseField(field);
            break;
        case 4:
            daysOfWeek = parseField(field);
            break;
        }
    }
    if (extraConfig.empty())
        std::getline(ss, extraConfig);
}

std::vector<int> ScheduledTask::parseField(const std::string &field)
{
    std::vector<int> values;
    if (field == "*")
    {
        // Handle the wildcard
        // For simplicity, we're not handling steps like */5
        return values; // An empty vector is a wildcard
    }
    std::istringstream ss(field);
    std::string value;
    while (std::getline(ss, value, ','))
    {
        values.push_back(std::stoi(value));
    }
    return values;
}

bool ScheduledTask::matches(int timeValue, const std::vector<int> &values)
{
    if (values.empty())
        return true; // Wildcard
    for (int value : values)
    {
        if (timeValue == value)
            return true;
    }
    return false;
}

std::string ScheduledTask::createExecutionID(const std::tm &time)
{
    std::ostringstream oss;

    // Minute
    if (!minutes.empty() && minutes[0] != -1)
    {
        oss << "M" << time.tm_min;
    }

    // Hour
    if (!hours.empty() && hours[0] != -1)
    {
        oss << "H" << time.tm_hour;
    }

    // Day of Month
    if (!daysOfMonth.empty() && daysOfMonth[0] != -1)
    {
        oss << "D" << time.tm_mday;
    }

    // Month
    if (!months.empty() && months[0] != -1)
    {
        oss << "Mo" << (time.tm_mon + 1); // tm_mon is 0-11
    }

    // Day of Week
    if (!daysOfWeek.empty() && daysOfWeek[0] != -1)
    {
        oss << "W" << time.tm_wday; // tm_wday is 0-6, Sunday = 0
    }

    // If oss is empty, it means all fields are wildcards, so add minute
    if (oss.str().empty())
    {
        oss << "M" << time.tm_min;
    }

    return oss.str();
}