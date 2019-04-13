#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

/*
** Our own implementation of a blocking queue, in which calls to pop() will
** block while the queue is empty. It is useful for queueing things to be sent
** over sockets, both on the server and client. This **should** be threadsafe,
** but it has not been thoroughly tested.
*/

template<typename T>
class BlockingQueue
{
private:
    // Backed by an STL queue
	std::queue<T> _queue;
	std::mutex _mutex;
	std::condition_variable _cond;
public:
    /*
    ** Pushes an object onto the queue. Signals via the _cond variable.
    */
	void push(T & item)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_queue.push(item);
		lock.unlock();
		_cond.notify_one();
	};

    /*
    ** Removes an item from the queue and returns it by reference. If the
    ** queue is empty, pop() will block until something has been added.
    */
	void pop(T & item)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		while (_queue.empty())
		{
			_cond.wait(lock);
		}
		item = _queue.front();
		_queue.pop();
	};

    /*
    ** Simple check to see if the queue is empty or not.
    */
	bool isEmpty()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _queue.empty();
	};
};

