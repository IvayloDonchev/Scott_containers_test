#include "wintimer.h"
#include <vector>
#include <iostream>
using namespace std;

int main() {
	constexpr auto nanoSecondsPerSecond = 1'000'000'000;

	std::cout << "ticksPerSecond   = " << WinTimer::ticksPerSecond()
		<< " (1 tick = " << nanoSecondsPerSecond / WinTimer::ticksPerSecond() << "ns)"
		<< endl;
	WinTimer wt;
	vector<int> dv;
	wt.reset();
	for (size_t i = 1; i <= 100000; ++i)
		dv.push_back(i);
	auto ticks = wt.ticks();
	cout << "ticks: " << ticks << endl;
	cout << "seconds: " << ticks / wt.ticksPerSecond() << endl;
	
}
