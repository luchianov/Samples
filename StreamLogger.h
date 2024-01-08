#pragma once
#include <Arduino.h>
#include <BluetoothSerial.h>

// Enumeration for log channels
enum class LogChannel
{
    NONE = 0,
    SERIAL_CHANNEL = 1,
    BT_CHANNEL = 2
};

// StreamLogger class definition
class StreamLogger
{
public:
    HardwareSerial &serial_port;
    BluetoothSerial &bt_serial;
    LogChannel current_channel;

    // TODO: introduce the level of verboseness, later

    StreamLogger(HardwareSerial &serial, BluetoothSerial &bt)
        : serial_port(serial), bt_serial(bt), current_channel(LogChannel::SERIAL_CHANNEL) {}

    void configure_channel(LogChannel channel)
    {
        current_channel = channel;
    }

    Stream &get_channel()
    {
        if (current_channel == LogChannel::BT_CHANNEL)
            return bt_serial;
        else
            return serial_port;
    }

    // Generic print method using templates
    template <typename T>
    void print(const T &message)
    {
        switch (current_channel)
        {
        case LogChannel::SERIAL_CHANNEL:
            serial_port.print(message);
            break;
        case LogChannel::BT_CHANNEL:
            bt_serial.print(message);
            break;
        case LogChannel::NONE:
        default:
            // No logging
            break;
        }
    }

    // Generic println method using templates
    template <typename T>
    void println(const T &message)
    {
        switch (current_channel)
        {
        case LogChannel::SERIAL_CHANNEL:
            serial_port.println(message);
            break;
        case LogChannel::BT_CHANNEL:
            bt_serial.println(message);
            break;
        case LogChannel::NONE:
        default:
            // No logging
            break;
        }
    }

    // Overload of println without any parameters
    void println()
    {
        switch (current_channel)
        {
        case LogChannel::SERIAL_CHANNEL:
            serial_port.println();
            break;
        case LogChannel::BT_CHANNEL:
            bt_serial.println();
            break;
        case LogChannel::NONE:
        default:
            // No logging
            break;
        }
    }
    // Print formatted string
    void printf(const char *format, ...)
    {
        char buffer[1024]; // Define the size as per your requirement
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        switch (current_channel)
        {
        case LogChannel::SERIAL_CHANNEL:
            serial_port.print(buffer);
            break;
        case LogChannel::BT_CHANNEL:
            bt_serial.print(buffer);
            break;
        case LogChannel::NONE:
        default:
            // No logging
            break;
        }
    }
};

// Global logger instance declaration
extern StreamLogger stream_logger;
