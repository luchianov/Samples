/**
 * @file ScheduleManager.h
 * @author Slava Luchianov
 * @brief This file contains a scheduler related functionality
 * It allows storing the information when the system shall start working
 * and when it should stop working.
 * The scheduling information is stored in the flash memory to survive over the power outage
 * When the system is rebooted or reset, the scheduler inspects the flash memory
 * trying to restore its content. After it retreives the rechedule records, it starts the tasks
 * to start and stop the playing of the paths.
 *
 * We are going to use a Unix cron-style scheduling system for our project. Cron is a time-based job scheduler in Unix-like operating systems, and it uses a specific format to represent different time intervals. The format consists of five fields, representing different units of time, and it looks like this:

* * * * *
| | | | |
| | | | +---- Day of the Week   (0 - 7) [0 and 7 are Sunday]
| | | +------ Month of the Year (1 - 12)
| | +-------- Day of the Month  (1 - 31)
| +---------- Hour              (0 - 23)
+------------ Minute            (0 - 59)

Each field can have:

    A number in its range.
    A star * which represents all possible values for that field.
    A list of numbers or ranges separated by commas, e.g., 1,3,5 or 1-3.
        We will not support the ranges
    Increments like *<backshash>15 in the minutes field to represent every 15 minutes.
        We will probably not support the "every n minutes" format

Here's a quick breakdown:

    5 * * * * would mean "run at 5 minutes past every hour."
    0 2 * * * means "run at 2:00 AM every day."
    0 0 * * 0 means "run at midnight on every Sunday."
    *<backshash>10 * * * * means "run every 10 minutes."
 *
 * It should probably support the Command Processor commands (TBD)
 * cmd_add_time_range(start_time, end_time)
 * cmd_list_time_ranges() - returns a list of ranges
 * cmd_delete_time_range(n)
 * Or maybe some other strategies.
 *
 * @version 0.1
 * @date 2023-11-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once
#include <vector>
#include <memory>
#include <iostream>

#include "SPIFFS.h"

#include "StreamLogger.h"
#include "AlgoHelper.h"
#include "ScheduledTask.h"

class ScheduleManager
{
public:
    ScheduleManager(){};
    ScheduleManager(std::string listOfTasks);

    bool loadTasks(std::string listOfTasks);
    void addTask(const std::string &schedule, const std::string &config = "");
    void deleteTask(int index);
    void deleteAllTasks();
    void listTasks();
    void checkAndRunTasks(std::function<bool(std::string &)> commandProcessorFunc);
    void saveToSpiffs();
    void restoreFromSpiffs();
    bool delayed_setup();

private:
    std::vector<std::unique_ptr<ScheduledTask>> tasks;
};

extern ScheduleManager schedule_manager;