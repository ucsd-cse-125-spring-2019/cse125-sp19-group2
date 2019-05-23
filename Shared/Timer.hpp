#pragma once

#include <functional>
#include <chrono>

/** Utility class for use on the server or client. These are "fake" timers in
  * that they are synchronous and the update() function needs to be called on
  * them frequently. This is enough for our needs in this case, because we
  * have very fast loops on both the server and client.
  */
class Timer {
public:
	// Constructs a timer object with a duration and an optional lambda to be
	// executed when the duration is reached. Duration is in milliseconds.
	// Timer is started immediately upon construction.
	Timer(long durationMilliseconds, std::function<void()> f = 0)
	{
		// Convert milliseconds to nanoseconds
		_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::milliseconds(durationMilliseconds));

		_onComplete = f;

		_durationReached = false;

		// Start the timer
		_startTime = std::chrono::steady_clock::now();
	}

	// Must be called every tick
	void update()
	{
		if (!_durationReached)
		{
			auto elapsed = std::chrono::steady_clock::now() - _startTime;
			if (elapsed >= _duration)
			{
				_durationReached = true;

				// Call lambda
				_onComplete();
			}
		}
	}

	// Stops and marks the timer for deletion
	void abort()
	{
		_durationReached = true;
	}

	// Has the timer reached its duration?
	bool isComplete()
	{
		update();
		return _durationReached;
	}

	// Returns elapsed time in milliseconds
	long getElapsed()
	{
		update();
		auto elapsed = std::chrono::steady_clock::now() - _startTime;
		return (long)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
	}

private:
	std::chrono::time_point<std::chrono::steady_clock> _startTime;
	std::chrono::nanoseconds _duration;
	std::function<void()> _onComplete;
	bool _durationReached;
};
