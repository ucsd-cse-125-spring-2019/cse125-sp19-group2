#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <ctime>

class Logger
{
public:
    static Logger * getInstance()
    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (_instance == NULL)
        {
            _instance = new Logger();
        }
        return _instance;
    }

    // Log with severity "DEBUG"
    void debug(std::string str)
    {
        printFormattedMessage("DEBUG", str);
    }

    // Log with severity "INFO"
    void info(std::string str)
    {
        printFormattedMessage("INFO", str);
    }

    // Log with severity "WARNING"
    void warn(std::string str)
    {
        printFormattedMessage("WARNING", str);
    }

    // Log with severity "ERROR"
    void error(std::string str)
    {
        printFormattedMessage("ERROR", str);
    }

    // Log with severity "FATAL"
    void fatal(std::string str)
    {
        printFormattedMessage("FATAL", str);
    }

private:
    static Logger * _instance;
    static std::mutex _mutex;
    static std::ostream* _os;

    Logger()
    {
        // Set desired output stream here
        _os = &std::cerr;
    };

    // Returns a formatted time string
    std::string getCurTime()
    {
        std::time_t curtime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        char buf[26];
        ctime_s(buf, sizeof(buf), &curtime);
        buf[24] = 0; 
        return std::string(buf);
    }

    // Prints a formatted log message
    void printFormattedMessage(std::string severity, std::string message)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        *_os << "[" << getCurTime() << "][" << severity << "] " << message << std::endl;
    }
};

