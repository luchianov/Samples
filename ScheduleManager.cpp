/**
 * @file ScheduleManager.cpp
 * @author Slava Luchianov
 * @brief
 * @version 0.1
 * @date 2023-12-14
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "ScheduleManager.h"

ScheduleManager::ScheduleManager(std::string listOfTasks)
{
    loadTasks(listOfTasks);
}

bool ScheduleManager::loadTasks(std::string listOfTasks)
{
    std::istringstream stream(listOfTasks);
    std::string line;

    while (std::getline(stream, line, '|'))
    {
        if (!line.empty())
        {
            addTask(line);
        }
    }
    return true;
}

void ScheduleManager::addTask(const std::string &schedule, const std::string &config)
{
    if (!config.empty())
    {
        tasks.push_back(std::make_unique<ScheduledTask>(schedule, config));
    }
}

void ScheduleManager::deleteTask(int index)
{
    if (index >= 0 && index < tasks.size())
    {
        tasks.erase(tasks.begin() + index);
    }
}

void ScheduleManager::deleteAllTasks()
{
    tasks.clear();
}

void ScheduleManager::listTasks()
{
    for (int i = 0; i < tasks.size(); ++i)
        stream_logger.printf("Task #%d, schedule: %s, config: %s\n", i,
                             tasks[i]->getSchedule().c_str(), tasks[i]->getConfig().c_str());
}

void ScheduleManager::checkAndRunTasks(
    std::function<bool(std::string &)> commandProcessorFunc)
{
    // stream_logger.println("ScheduleManager::checkAndRunTasks()");
    static uint8_t schedulerIterations = 0;
    if (++schedulerIterations % 10 == 0)
    {
        stream_logger.printf("S");
        schedulerIterations = 0;
    }

    for (auto &task : tasks)
    {
        if (task->shouldRunNow())
        {
            std::string taskConfig = task->getConfig();
            stream_logger.printf("Schedule: %s, command: %s\n", task->getSchedule().c_str(), taskConfig.c_str());
            commandProcessorFunc(taskConfig);
        }
    }
}

void ScheduleManager::saveToSpiffs()
{
    stream_logger.println("ScheduleManager::saveToSpiffs()");
#if false
    for (auto &task : tasks)
    {
        printf((task->getSchedule() + " |" + task->getConfig()).c_str());
    }
#else
    File file = SPIFFS.open("/crontab", FILE_WRITE);
    if (!file)
    {
        stream_logger.println("Failed to open crontab file for writing");
        return;
    }

    for (auto &task : tasks)
    {
        file.println((task->getSchedule() + " |" + task->getConfig()).c_str());
    }
    file.close();
#endif
}

void ScheduleManager::restoreFromSpiffs()
{
    stream_logger.println("ScheduleManager::restoreFromSpiffs()");
#if false
    while (true)
    {
        std::string line = "* 2,3 3 4 * Hello";
        size_t pipeDivider = line.find_last_of('|');
        std::string schedule = line.substr(0, pipeDivider);
        std::string config = line.substr(pipeDivider + 1);
        tasks.push_back(std::make_unique<ScheduledTask>(schedule));
        break;
    }
#else
    File file = SPIFFS.open("/crontab", FILE_READ);
    if (!file)
    {
        stream_logger.println("Failed to open crontab file for reading");
        return;
    }
    else
        stream_logger.println("Crontab file open for reading");

    tasks.clear();

    while (file.available())
    {
        String line = file.readStringUntil('\n');
        size_t pipeDivider = line.lastIndexOf('|');
        std::string schedule = line.substring(0, pipeDivider).c_str();
        std::string config = line.substring(pipeDivider + 1).c_str();
        tasks.push_back(std::make_unique<ScheduledTask>(schedule, config));
        stream_logger.printf("Crontab: %s %s\n", schedule.c_str(), config.c_str());
    }
    file.close();
    this->listTasks();
#endif
}

bool ScheduleManager::delayed_setup()
{
    restoreFromSpiffs();
    return true;
}
