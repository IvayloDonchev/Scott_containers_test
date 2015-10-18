// #include "stdafx.h"                          // keep visual studio happy

#if defined(_MSC_VER)                           // prevent <windows.h> from defining macros named,
  #define NOMINMAX                              // of all things, min and max
#endif

#include <iostream>
#include <cstdio>                               // for formatted IO
#include <fstream>
#include <algorithm>
#include <limits>
#include <random>
#include <array>
#include <vector>
#include <string>
#include <tuple>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <chrono>


// From Jonathan Wakely (https://gist.github.com/jwakely/2ca40c29c14dd6e30132)
class Timer {
public:

    // create and start timer
    Timer() { reset(); }

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


// sets hold KeyTypes, maps hold (KeyType, MappedType) pairs
using KeyType = unsigned long;
using MappedType = void*;

constexpr auto MaxKeyValue = std::numeric_limits<KeyType>::max();

constexpr std::size_t one = 1;                        // 1 as a std::size_t
constexpr std::size_t maxContainerSize = 100;
constexpr bool testingUnsortedVector = maxContainerSize <= 100;
constexpr std::size_t timingsPerSize = 50;
constexpr std::size_t containerSizeStep = std::max(one, maxContainerSize / timingsPerSize);


// return a random int in range [0, MaxKeyValue].
auto randomValue()
{
    static std::mt19937 eng;
    static std::uniform_int_distribution<KeyType> dis(0, MaxKeyValue);

    return dis(eng);
}


constexpr std::size_t lookupsToPerform = 1'000'000;
std::array<KeyType, lookupsToPerform> valuesToLookUp;


// Return ticks taken (total and average) to invoke f on each entry in valuesToLookUp.
// Loop overhead time is included.
using TickAverageType = long double;
using LookupTimeDataType = std::tuple<decltype(std::declval<Timer>().ticks()), TickAverageType>;

enum { TotalTicksIdx, AvgTicksIdx };   // for indexing into
                                       // LookupTimeDataType objects
 
// revised by Tomasz Kamiński to defeat loop-eliminating compiler optimizations
template<typename LookupFuncType>
auto doAndTimeLookups(const LookupFuncType& f)
{
    Timer t;                        // start timer

    volatile std::size_t count = 0;
    for (auto val : valuesToLookUp) count += f(val);

    auto ticks = t.ticks();            // stop timer
    return std::make_tuple(ticks, static_cast<TickAverageType>(ticks) / valuesToLookUp.size());
}

constexpr auto separatorChar = '\t';
constexpr auto newLine = '\n';


int main()
{
    constexpr auto nanoSecondsPerSecond = 1'000'000'000;

    std::cout << "ticksPerSecond   = " << Timer::ticksPerSecond()
              << " (1 tick = " << nanoSecondsPerSecond / Timer::ticksPerSecond() << "ns)" 
              << newLine;
    std::cout << "lookupsToPerform = " << valuesToLookUp.size() << newLine;
    std::cout << "maxContainerSize = " << maxContainerSize << newLine;
    std::cout << "testingUnsortedVector = " << std::boolalpha << testingUnsortedVector << newLine;
    std::cout << newLine;

#if defined(__GNUC__)
  #define FILE_NAME_PREFIX "GCC"
#elif (_MSC_VER == 1900)
  #define FILE_NAME_PREFIX "MS"
#endif

    const std::string timingsFileName(FILE_NAME_PREFIX "_lookupTimings.dat");

    std::ofstream timingsFile(timingsFileName);
    if (!timingsFile) {
        std::cerr << "Unable to open timings file: " << timingsFileName << newLine;
        std::exit(EXIT_FAILURE);
     }

    // write column headings into data file
    timingsFile << "Container Size" << separatorChar
                << "std::set" << separatorChar
                << "std::map" << separatorChar
                << "std::unordered_set" << separatorChar
                << "std::unordered_map" << separatorChar
                << "boost::flat_set" << separatorChar
                << "boost::flat_map";
    if (testingUnsortedVector) timingsFile << separatorChar << "unsorted std::vector";
    timingsFile << newLine;

    for (std::size_t containerSize = containerSizeStep;
         containerSize <= maxContainerSize; 
         containerSize += containerSizeStep) {

        // create collections with containerSize unique random entries
        std::set<KeyType> set;
        std::map<KeyType, MappedType> map;
        std::unordered_set<KeyType> unorderedSet;
        std::unordered_map<KeyType, MappedType> unorderedMap;
        std::vector<KeyType> unsortedVector;

        while (set.size() < containerSize)
        {
            auto r = randomValue();
            if (set.insert(r).second == true)
            {                                    // entry is new, so add to all other collections
                map[r] = nullptr;
                unorderedSet.insert(r);
                unorderedMap[r] = nullptr;
                if (testingUnsortedVector) unsortedVector.push_back(r);
            };
        };

        boost::container::flat_set<KeyType> flatSet(set.cbegin(), set.cend());
        boost::container::flat_map<KeyType, MappedType> flatMap(map.cbegin(), map.cend());

        // fill valuesToLookUp with random values
        std::generate(valuesToLookUp.begin(), valuesToLookUp.end(), randomValue);

        // time the lookups
        LookupTimeDataType ticksForSet = doAndTimeLookups([&](auto val) { return set.find(val) != set.end(); });
        LookupTimeDataType ticksForMap = doAndTimeLookups([&](auto val) { return map.find(val) != map.end(); });
        LookupTimeDataType ticksForUnorderedSet = doAndTimeLookups([&](auto val) { return unorderedSet.find(val) != unorderedSet.end(); });
        LookupTimeDataType ticksForUnorderedMap = doAndTimeLookups([&](auto val) { return unorderedMap.find(val) != unorderedMap.end(); });
        LookupTimeDataType ticksForFlatSet = doAndTimeLookups([&](auto val) { return flatSet.find(val) != flatSet.end(); });
        LookupTimeDataType ticksForFlatMap = doAndTimeLookups([&](auto val) { return flatMap.find(val) != flatMap.end(); });
        LookupTimeDataType ticksForUnsortedVector;
        if (testingUnsortedVector) {
          ticksForUnsortedVector = doAndTimeLookups([&](auto val) { return std::find(unsortedVector.begin(),
                                                                                     unsortedVector.end(),
                                                                                     val) != unsortedVector.end(); });
        }

        std::cout << "Writing data for containerSize = " << containerSize << "..." << newLine;

        timingsFile << containerSize << separatorChar 
                    << std::get<AvgTicksIdx>(ticksForSet) << separatorChar
                    << std::get<AvgTicksIdx>(ticksForMap) << separatorChar
                    << std::get<AvgTicksIdx>(ticksForUnorderedSet) << separatorChar 
                    << std::get<AvgTicksIdx>(ticksForUnorderedMap) << separatorChar 
                    << std::get<AvgTicksIdx>(ticksForFlatSet) << separatorChar 
                    << std::get<AvgTicksIdx>(ticksForFlatMap);
        if (testingUnsortedVector) timingsFile << separatorChar << std::get<AvgTicksIdx>(ticksForUnsortedVector);
        timingsFile << newLine;
    }

    timingsFile.close();
}
