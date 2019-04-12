#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class BlockingQueue
{
private:
	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _cond;
public:
	void push(T & item)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_queue.push(item);
		lock.unlock();
		_cond.notify_one();
	};

	void pop(T & item)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (_queue.empty())
		{
			_cond.wait(lock);
		}
		item = _queue.front();
		_queue.pop();
	}
};

