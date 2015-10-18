#pragma once
#include <chrono>

class WinTimer {
public:

	// create and start timer
	WinTimer() { reset(); }

	// return ticks since timer creation or last reset
	auto ticks() const
	{
		return (clock::now() - mTimeStart).count();
	}

	void reset() { mTimeStart = clock::now(); }

	static auto ticksPerSecond()
	{
		double den = clock::duration::period::den;
		return den / clock::duration::period::num;
	}

private:
	using clock = std::chrono::high_resolution_clock;
	clock::time_point mTimeStart;
};