/**
 * Usage:
 *
 * Calculate execution time of continuous code (in miliseconds):
 *      Statistic myStatistic;
 *      ...
 *      START_TICK_COUNT(myStatistic, "Code name for report");
 *      std::sort(collection.begin(), collection.end());
 *      STOP_TICK_COUNT(myStatistic, "Code name for report");
 *
 * Calculate execution time of sparse code (in miliseconds):
 *      Statistic myStatistic;
 *      ...
 *      BEGIN_CALC_SESSION(myStatistic, "Code name for report");
 *      ...
 *      START_TICK_COUNT(myStatistic, "Code name for report");
 *      std::sort(collection.begin(), collection.end());
 *      STOP_TICK_COUNT(myStatistic, "Code name for report");
 *      ...
 *      START_TICK_COUNT(myStatistic, "Code name for report");
 *      std::sort(collection.begin(), collection.end());
 *      STOP_TICK_COUNT(myStatistic, "Code name for report");
 *      ...
 *      END_CALC_SESSION(myStatistic, "Code name for report");
 *
 * Add value to counter:
 *      Statistic myStatistic;
 *      ...
 *      INCREASE_STAT_VALUE(myStatistic, "Code name for report", 100500);
 */

#pragma once

#include <noncopyable.h>

#include <sys/time.h>

#include <thread>
#include <unordered_map>

#ifdef STAT

#define BEGIN_CALC_SESSION(stat, name) stat.beginSession(name);
#define END_CALC_SESSION(stat, name) stat.endSession(name);
#define START_TICK_COUNT(stat, name) stat.startTickCount(name);
#define STOP_TICK_COUNT(stat, name) stat.stopTickCount(name);
#define INCREASE_STAT_VALUE(stat, name, value) stat.increaseCounterValue(name, value);

#else

#define BEGIN_CALC_SESSION(stat, name)
#define END_CALC_SESSION(stat, name)
#define START_TICK_COUNT(stat, name)
#define STOP_TICK_COUNT(stat, name)
#define INCREASE_STAT_VALUE(stat, name, value)

#endif

size_t getTimeMs() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((t.tv_sec) * 1000 + t.tv_usec/1000.0) + 0.5;
}

static __inline__ unsigned long long rdtsc() {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}


class Statistic : Noncopyable {
public:
    void beginSession(const std::string& aName) {
        std::unique_lock<std::mutex> lock(mutex);
        beginSessionUnlocked(aName);
    }

    void endSession(const std::string& aName) {
        std::unique_lock<std::mutex> lock(mutex);
        endSessionUnlocked(aName);
    }

    void startTickCount(const std::string& aName) {
        std::unique_lock<std::mutex> lock(mutex);
        size_t startTime = rdtsc();

        StatisticData& data = statisticMap[aName];

        beginSessionUnlocked(aName);

        data.startTime = startTime;
    }

    void stopTickCount(const std::string& aName) {
        size_t stopTime = rdtsc();

        mutex.lock();

        StatisticData& data = statisticMap[aName];
        data.stopTime = stopTime;

        float execTime = (data.stopTime - data.startTime) / float(CLOCKS_PER_SEC);
        data.lastExecuteTime += execTime;

        data.total += execTime;

        endSessionUnlocked(aName);

        mutex.unlock();
    }

    /**
     * Average execution time of value aName
     */
    float getAverageTime(const std::string& aName) {
        std::unique_lock<std::mutex> lock(mutex);
        return statisticMap[aName].avg;
    }

    float getTotalTime(const std::string& aName) {
        std::unique_lock<std::mutex> lock(mutex);
        return statisticMap[aName].total;
    }

    /**
     * All average execution times
     */
    std::vector<std::pair<std::string, float> > getAllAverages() const {
        std::unique_lock<std::mutex> lock(mutex);
        std::vector< std::pair<std::string, float> > result;

        auto it = statisticMap.begin();
        for (; it != statisticMap.end(); ++it) {
            result.push_back(std::make_pair(it->first, it->second.avg));
        }

        return result;
    }

    void increaseCounterValue(const std::string& aName, std::size_t aValue) {
        std::unique_lock<std::mutex> lock(mutex);
        countsMap[aName] += aValue;
    }

    std::size_t getCounterValue(const std::string& aName) {
        std::unique_lock<std::mutex> lock(mutex);
        return countsMap[aName];
    }

    std::vector< std::pair<std::string, std::size_t> >&& getAllCounterValues() {
        std::unique_lock<std::mutex> lock(mutex);
        std::vector< std::pair<std::string, std::size_t> > result;

        auto it = countsMap.begin();
        for (; it != countsMap.end(); ++it) {
            result.push_back(*it);
        }

        return std::move(result);
    }

    void clear() {
        std::unique_lock<std::mutex> lock(mutex);
        statisticMap.clear();
        countsMap.clear();
    }

private:
    void beginSessionUnlocked(const std::string& aName) {
        StatisticData& data = statisticMap[aName];

        // if first time start session
        if (data.sessionRefCount == 0) {
            data.lastExecuteTime = 0;
            data.startTime = 0;
            data.stopTime = 0;
        }
        ++data.sessionRefCount;
    }

    void endSessionUnlocked(const std::string& aName) {
        StatisticData& data = statisticMap[aName];

        assert(data.sessionRefCount > 0);

        // if there is another session holders
        if (--data.sessionRefCount > 0) {
            return;
        }

        data.avg = (data.avg * data.count + data.lastExecuteTime) / (data.count + 1);

        ++data.count;
    }

private:
    struct StatisticData {
        StatisticData() :
            count(0), avg(.0f), total(.0f), startTime(0), stopTime(0),
            lastExecuteTime(.0f), sessionRefCount(0) {}

        size_t count;
        float avg;
        float total;
        size_t startTime;
        size_t stopTime;
        float lastExecuteTime;
        size_t sessionRefCount;
    };

    std::unordered_map<std::string, StatisticData> statisticMap;
    std::unordered_map<std::string, std::size_t> countsMap;
    mutable std::mutex mutex;
};
