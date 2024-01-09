# ESP32 Controller Project Components

This repository contains a collection of key components for an ESP32-based controller system. These components are designed to work together to provide a robust and flexible firmware base capable of handling complex tasks. 

## Main.cpp

The entry point for the ESP32 firmware, setting up essential hardware and software components and entering the main loop where task checking and execution takes place.

- Employs Bluetooth and/or Serial channel to communicate with the client.
- Implements singleton pattern for efficient resource management.
- Showcases integration of multiple system components and task scheduling.

## ScheduledTask

Defines the `ScheduledTask` class, managing individual task schedules and execution.

- Implements complex scheduling logic with efficiency and reliability.
- Incorporates unique execution IDs to prevent duplicate task runs.

## ScheduleManager

Manages a collection of `ScheduledTask` instances, handling task addition, deletion, and execution based on Unix cron-style scheduling.

- Relies upon standard Linux CRON scheduling specifications to fit specific project requirements.
- Balances functionality and simplicity in design and implementation.
- Employs dependency injection pattern to uncouple from the CommandProcessor.
- Leverages persistent storage for schedule integrity across system restarts.

## StreamLogger.h

Provides a logging interface to aid in debugging and monitoring the system's behavior.

- Facilitates clear and consistent logging throughout the system.
- Supports Bluetooth/Serial communication channel configuration.
- Integrates seamlessly with other system components for in-depth diagnostics.

## ClockHelper

Handles time synchronization and provides utility functions for time management within the ESP32 environment.

- Ensures accurate timekeeping for task scheduling and logging.
- Showcases efficient use of the ESP32's hardware features for time-related functions.
- Integrates with the RTC for maintaining time across power cycles.

## CommandPlayer (Program.cs) 

A small C# client program which communicates with the ESP32 firmware over Bluetooth.

- Communicates over Bluetooth with ESP32, sending the commands and receiving responses.
- Captures and presents the logging information from the Controller firmware processing the commands.

## Contribution

The project is closed for contributions.

## License

This code repo is licensed under the GNU GENERAL PUBLIC LICENSE - see the LICENSE file for details.

