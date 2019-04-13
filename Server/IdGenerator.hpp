#pragma once

#include <stdint.h>
#include <iostream>
#include <mutex>

/*
** ID generator class. Is a singleton, so can be called anywhere from within
** the server code. Use it to generate an ID for anything that requires state
** to be synced with clients. It will never generate an ID of zero.
*/

class IdGenerator
{
private:
	static IdGenerator * _instance;
    static std::mutex _mutex;
	uint32_t _nextId;

	IdGenerator()
	{
        _nextId = 1;
	}

public:
	
	static IdGenerator * getInstance()
	{
        std::unique_lock<std::mutex> lock(_mutex);
		if (!_instance)
		{
			_instance = new IdGenerator();
		}
		return _instance;
	}
		
	uint32_t getNextId()
	{
        std::unique_lock<std::mutex> lock(_mutex);
        if ((_nextId + 1) < _nextId)
        {
            std::cerr << "IDs are starting to wrap around. Shutting down server."
                    << std::endl;
            fgetc(stdin);
            exit(1);
        }
        else
        {
            return _nextId++;
        }
	}
};

