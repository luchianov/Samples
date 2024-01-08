/**
 * @file main.cpp
 * @author Slava Luchianov
 * @date 2023-11-01
 * @brief The main.cpp for ESP32 contains the two most important methods setup() and loop()
 *
 * The setup() is executed once in the beginning of the execution,
 *    with the power_on event or after the reset event, after the crash or by the reset button.
 *
 * @section Section 1, Singletons
 * The application consists of a hierarchy of several singletons
 * PWM Adapter object takes care about controlling the servos
 * BT Serial object supports a duplex communication with the mobile device
 * Schedule Manager fixes the schedule turning the path player on/off to control the laser
 * Storage Manager supports (de)serialization of the Path Manager content in the flash storage
 *
 * @section Section 2, setup()
 * This method initializes the required components.
 * For example it starts the scheduler component
 *
 * @section Section 3, loop()
 * This method communicates with the clients over the Bluetooth or Serial channels.
 * It reads as much as it can and then passes the collected content into the command processor
 *
 */

#include <Arduino.h>
#include <ESP.h>
#include <BluetoothSerial.h>
#include "SPIFFS.h"

#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <functional>

#include "StreamLogger.h"
#include "AlgoHelper.h"
#include "ClockHelper.h"
#include "ServoAdapter.h"
#include "ServoController.h"
#include "CommandProcessor.h"
#include "ScheduleManager.h"
#include "LaserHelper.h"

// TODO: change cross the project from the String to the std::string
String input_string;         // A string to hold incoming data
bool stringComplete = false; // Whether the string is complete
long iterations = 0;

// Create all your shared singletons here to pass them into the CommandProcessor constructor later
HardwareSerial &default_serial = Serial; // Reference to the default hardware serial
BluetoothSerial bt_serial;

// Global logger instance definition
StreamLogger stream_logger(default_serial, bt_serial);

// Hardware controlling hierarchy instantiation
LaserHelper laser_helper;
ServoAdapter servo_x;
ServoAdapter servo_y;
ServoController servo_controller;
ScheduleManager schedule_manager;
CommandProcessor command_processor;
ClockHelper runtime_clock_helper;

void setup()
{
    // TODO: rename the registered name to something like "SmartLaserController" or LaserCatnip :)
    bt_serial.begin("ESP32");
    default_serial.begin(115200); // Initialize serial communication at 115200 baud
    stream_logger.println("main.cpp.setup(): listen to BT and Serial, log the response");

    laser_helper.delayed_setup();
    runtime_clock_helper.delayed_setup();

    //  Initialize SPIFFS
    if (!SPIFFS.begin(true))
        stream_logger.println("An Error has occurred while mounting SPIFFS");

    // Let's restore the working schedule from the flash memory, to know when to turn it on/off
    schedule_manager.delayed_setup();

    // Let's restore all the paths and the scale info from the flash memory
    path_manager.delayed_setup();

#ifndef PROD
    // Starting the player
    std::string startPlayerCommand = "{\"command\":\"path_player_switch\",\"player\":\"on\"}";
    command_processor.process_command(startPlayerCommand);

    stream_logger.printf("setup() running on core # %d\n", xPortGetCoreID());
    stream_logger.printf("Flash memory size: %d bytes\n", ESP.getFlashChipSize());
#endif

    input_string.reserve(1000); // Reserve 1000 bytes for the inputString
}

/**
 * @brief Define an std::function lambda that binds to process_command method
 * of the command_processor to make the schedule_manager.checkAndRunTasks(...lambda...) method
 * happy (many thanks to Chat GPT)
 */
auto processCommandFunc = [](std::string &config) -> bool
{
    return command_processor.process_command(config);
};

void loop()
{
    runtime_clock_helper.synchronize_esp32_to_rtc_at_24_hours();

    if (iterations % 1000 == 0) // every 1 seconds
        schedule_manager.checkAndRunTasks(processCommandFunc);

    // Print a heartbeat dot "\n" every 1min, which takes care about the recurrent garbage in the Serial channel, which we noticed when no 'line break' was in the channel
    if (iterations % 60000 == 0)
        stream_logger.printf("\n");
    // Print a heartbeat dot "." every 50 seconds
    if (iterations++ % 5000 == 0)
        stream_logger.print(".");
    if (iterations >= LONG_MAX - 10)
        iterations = 0;

    // Check for serial input as an alternative method to feed the commands into ESP32
    while (Serial.available())
    {
        char inChar = (char)Serial.read(); // Read a character from the serial port
        Serial.write(inChar);              // an echo to see what we actually type into Serial
        input_string += inChar;            // Append it to the input string
        if (inChar == '\n')
        { // If it is a newline, set a flag so the main loop can process it
            stringComplete = true;
            break;
        }
    }

    // Check for incoming BT data
    while (bt_serial.available())
    {
        char inChar = bt_serial.read();
        Serial.print(inChar);   // Echo like output from BT to Serial
        input_string += inChar; // Append it to the input string
        if (inChar == '\n')
        { // If the incoming character is a newline, set a flag so the main loop can process it
            stringComplete = true;
            break;
        }
    }
    vTaskDelay(1); // param is the delay in ticks until the next control cycle
    // If a complete string from the serial port has been received
    if (stringComplete)
    {
        runtime_clock_helper.time_stamp_to_serial();
        //  Example of sending a response back
        String response = "main.cpp.loop():\t Received a message: ";
        response.concat(input_string);
        stream_logger.print(response);
        bt_serial.print(response);
        std::vector<std::string> lines = AlgoHelper::split_string(input_string.c_str(), '\n');
        for (std::string line : lines)
        {
            if (line.length() > 1)
            {
                bool retCode = command_processor.process_command(line);
                stream_logger.printf("main.cpp.loop():\t The command processing returns %d \n\n",
                                     retCode);
                bt_serial.printf("main.cpp.loop():\t The command processing returns %d \n\n",
                                 retCode);
            }
        }

        input_string.clear();
        stringComplete = false; // Reset the flag
    }
}
