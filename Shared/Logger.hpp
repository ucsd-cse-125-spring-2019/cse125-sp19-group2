#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <ctime>
#include <vector>
#include <thread>
#include <chrono>
#include "Common.hpp"

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

	// Init utilization monitor thread
	void initUtilizationMonitor()
	{
		_utilizationThread = std::thread(
			&Logger::printUtilization,
			this);
	}

	// Store duration of single game loop
	void storeLoopDuration(long long duration)
	{
		std::unique_lock<std::mutex> lock(_durationMutex);
		_loopDurations.push_back(duration);
	}

private:
    static Logger * _instance;
    static std::mutex _mutex;
    static std::ostream* _os;

	// List of loop durations in microseconds, used by server only
	std::vector<long long> _loopDurations;
	std::mutex _durationMutex;

	// Thread to print utilization %, used by server only
	std::thread _utilizationThread;

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
		clearLine();
        *_os << "[" << getCurTime() << "][" << severity << "] " << message << std::endl;
    }

	// Clears the current line
	void clearLine();

	// Prints server utilization every few seconds; runs in separate thread
	void printUtilization()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));

			// Take average vector value
			long long totalUsage = 0;
			std::unique_lock<std::mutex> lock(_durationMutex);
			int durationCount = (int)_loopDurations.size();
			for (auto& duration : _loopDurations)
			{
				totalUsage += duration;
			}
			_loopDurations.clear();
			lock.unlock();

			// Acquire stderr lock and print utilization
			std::unique_lock<std::mutex> stderrLock(_mutex);

			// Clear current line first
			clearLine();
			*_os << "Utilization: " << (int)((float)(totalUsage / durationCount) / (std::pow(10, 6) / TICKS_PER_SEC) * 100) << "%";
		}
	}
};

