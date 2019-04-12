#pragma once
#include <stdint.h>
class IdGenerator
{
private:
	static IdGenerator * _instance;
	uint32_t _nextId;

	IdGenerator()
	{
		_nextId = 1;
	}

public:
	
	static IdGenerator * getInstance()
	{
		if (!_instance)
		{
			_instance = new IdGenerator();
		}
		return _instance;
	}
		
	uint32_t getNextId()
	{
		return _nextId++;
	}
};

