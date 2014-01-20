#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <future>
#include <list>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#define STAT

#include <statistic.h>

template <typename ItemType, typename InArchive>
class ReaderMtAdaptive {
    const size_t QUEUE_SIZE = 500000;

public:
    ReaderMtAdaptive(InArchive* inArch, ItemType* item, Statistic* stat) :
            inArchive(inArch),
            itemReadTo(item),
            statistic(stat),
            queueSize(QUEUE_SIZE),
            fillStarted(false),
            breakFuture(false),
            first(true) {
        fillQueue();
        waitData();
    }

    bool readNext() {
        Statistic& st = *statistic;

        //INCREASE_STAT_VALUE(st, "readNext", 1);

        //START_TICK_COUNT(st, "readNext");

        if (!inArchive->eof() && !fillStarted) {
            fillQueue();
        }

        //STOP_TICK_COUNT(st, "readNext");
        if (itemQueue.empty() && fillStarted) {
            //INCREASE_STAT_VALUE(st, "getDataTime", 1);
            //START_TICK_COUNT(st, "getDataTime");
            getDataFromReadThread();
            //STOP_TICK_COUNT(st, "getDataTime");
        }

        //START_TICK_COUNT(st, "readNext");
        bool eof = itemQueue.empty() && inArchive->eof() && !fillStarted;

        if (!eof) {
            assert(!itemQueue.empty());

            START_TICK_COUNT(st, "popData");
            //itemQueue.front();
            STOP_TICK_COUNT(st, "popData");
            *itemReadTo = std::move(itemQueue.front());
            itemQueue.pop();
        }

        //STOP_TICK_COUNT(st, "readNext");

        return !eof;
    }

    const ItemType& currentItem() const {
        return *itemReadTo;
    }

private:
    void getDataFromReadThread() {
        breakFuture = true;
        itemQueue = std::move(readFuture.get());
        assert(!itemQueue.empty());
        breakFuture = false;
        fillStarted = false;
        queueSize = itemQueue.size();
    }

    void fillQueue() {
        fillStarted = true;

        Statistic& st = *statistic;
        //START_TICK_COUNT(st, "startThread");
        readFuture = std::async(std::launch::async, [=]() -> ItemQueue {
            size_t readCount = 0;
            ItemQueue result;
            while (!inArchive->eof() && readCount++ < queueSize && !breakFuture) {
                ItemType item;
                deserialize(item, *inArchive);
                result.push(item);
            }

            first = false;

            return std::move(result);
        });
        //STOP_TICK_COUNT(st, "startThread");
    }

    void waitData() {
        itemQueue = std::move(readFuture.get());
        assert(!itemQueue.empty());
        fillStarted = false;
    }

private:
    InArchive* inArchive;
    ItemType* itemReadTo;
    typedef std::queue<ItemType> ItemQueue;
    ItemQueue itemQueue;
    ItemQueue tmpResult;
    std::future<ItemQueue> readFuture;
    Statistic* statistic;
    size_t queueSize;
    bool fillStarted;
    bool breakFuture;
    bool first;
};

template <typename ItemType, typename InArchive>
class ReaderMt {
    const size_t COUNT_THRESHOLD = 2500;
    const size_t QUEUE_SIZE = 5000;

public:
    ReaderMt(InArchive* inArch, ItemType* item, Statistic* stat) :
            inArchive(inArch),
            itemReadTo(item),
            fillStarted(false),
            statistic(stat) {
        fillQueue(500);
    }

    bool readNext() {
        Statistic& st = *statistic;

        //INCREASE_STAT_VALUE(st, "readNext", 1);

        //START_TICK_COUNT(st, "readNext");

        if (itemQueue.size() < COUNT_THRESHOLD && !inArchive->eof()) {
            if (!fillStarted) {
                fillQueue(QUEUE_SIZE);
            }
        }

        //STOP_TICK_COUNT(st, "readNext");
        if (itemQueue.empty() && fillStarted) {
            INCREASE_STAT_VALUE(st, "waitTime", 1);
            //START_TICK_COUNT(st, "waitTime");
            waitData();
            //STOP_TICK_COUNT(st, "waitTime");
        }

        //START_TICK_COUNT(st, "readNext");
        bool eof = itemQueue.empty() && inArchive->eof() && !fillStarted;

        if (!eof) {
            assert(!itemQueue.empty());

            *itemReadTo = std::move(itemQueue.front());
            itemQueue.pop();
        }

        //STOP_TICK_COUNT(st, "readNext");

        return !eof;
    }

    const ItemType& currentItem() const {
        return *itemReadTo;
    }

private:
    void fillQueue(size_t queueSize) {
        fillStarted = true;

        Statistic& st = *statistic;
        //START_TICK_COUNT(st, "startThread");
        readFuture = std::async(std::launch::async, [=]() -> ItemQueue {
            size_t readCount = 0;
            ItemQueue result;
            Statistic& st = *statistic;
            //START_TICK_COUNT(st, "readTime");
            while (!inArchive->eof() && readCount++ < queueSize) {
                ItemType item;
                deserialize(item, *inArchive);
                result.push(item);
            }
            //STOP_TICK_COUNT(st, "readTime");

            return std::move(result);
        });
        //STOP_TICK_COUNT(st, "startThread");
    }

    void waitData() {
        itemQueue = std::move(readFuture.get());
        assert(!itemQueue.empty());
        fillStarted = false;
    }

private:
    InArchive* inArchive;
    ItemType* itemReadTo;

    typedef std::queue<ItemType> ItemQueue;
    ItemQueue itemQueue;
    ItemQueue tmpResult;
    std::future<ItemQueue> readFuture;
    bool fillStarted;
    Statistic* statistic;
};


template <typename ItemType, typename InArchive>
class MtMerger {
    typedef ReaderMtAdaptive<ItemType, InArchive> Reader;

    class ItemHolder {
    public:
        explicit ItemHolder(Reader* rdr) :
                reader(rdr) {}

        bool operator < (const ItemHolder& other) const {
            // Make min heap
            return !(reader->currentItem() < other.reader->currentItem());
        }

        bool readNextItem() {
            return reader->readNext();
        }

        const ItemType& item() const {
            return reader->currentItem();
        }

    private:
        Reader* reader;
    };

    typedef std::priority_queue<ItemHolder> PriorityQueue;

public:
    explicit MtMerger(const std::list<InArchive>& archives) :
            inArchives(archives) {}

    template <typename ProcessFunction>
    void merge(ProcessFunction process) {
        PriorityQueue priorityQueue;

        std::vector<ItemType> itemMemory;
        itemMemory.reserve(inArchives.size());

        Statistic statistic;
        BEGIN_CALC_SESSION(statistic, "waitTime");
        BEGIN_CALC_SESSION(statistic, "startThread");
        BEGIN_CALC_SESSION(statistic, "readNext");
        BEGIN_CALC_SESSION(statistic, "popData");
        BEGIN_CALC_SESSION(statistic, "init");

        /*ItemType item;
        Reader reader(&inArchives.front(), &item, &statistic);
        while (reader.readNext()) {
        }

        return;*/

        //START_TICK_COUNT(statistic, "init");

        std::list<Reader> readers;

        typename std::list<InArchive>::iterator archIt = inArchives.begin();
        for (; archIt != inArchives.end(); ++archIt) {
            itemMemory.push_back(ItemType());

            readers.push_back(Reader(&*archIt, &itemMemory.back(), &statistic));

            ItemHolder holder(&readers.back());
            if (holder.readNextItem()) {
                priorityQueue.push(holder);
            }
        }

        //STOP_TICK_COUNT(statistic, "init");

        while (!priorityQueue.empty()) {
            ItemHolder holder = priorityQueue.top();
            priorityQueue.pop();

            process(holder.item());

            const bool queueEmpty = priorityQueue.empty();
            const ItemHolder& topHolder = priorityQueue.top();
            bool holderRead = holder.readNextItem();
            while (!queueEmpty && holderRead && holder.item() < topHolder.item()) {
                process(holder.item());
                holderRead = holder.readNextItem();
            }

            if (holderRead) {
                if (priorityQueue.empty()) {
                    do {
                        process(holder.item());
                    } while (holder.readNextItem());
                } else {
                    priorityQueue.push(holder);
                }
            }
        }

        /*START_TICK_COUNT(statistic, "popData");
        float t = 0;
        for (size_t i = 0; i < 1000000; ++i) {
            size_t t1 = rdtsc();
            size_t t2 = rdtsc();
            t += (t2 - t1) / float(CLOCKS_PER_SEC);
        }
        STOP_TICK_COUNT(statistic, "popData");

        std::cout << t << "\n";*/

        END_CALC_SESSION(statistic, "waitTime");
        END_CALC_SESSION(statistic, "startThread");
        END_CALC_SESSION(statistic, "readNext");
        END_CALC_SESSION(statistic, "popData");
        END_CALC_SESSION(statistic, "init");
        std::cout << "Start read thread time - " << statistic.getTotalTime("startThread") << "\n";
        std::cout << "Get data time - " << statistic.getTotalTime("getDataTime") << "\n";
        std::cout << "Get data count - " << statistic.getCounterValue("getDataTime") << "\n";
        std::cout << "Read next time - " << statistic.getTotalTime("readNext") << "\n";
        std::cout << "Read next count - " << statistic.getCounterValue("readNext") << "\n";
        std::cout << "Pop data time - " << statistic.getTotalTime("popData") << "\n";
        std::cout << "Init time - " << statistic.getTotalTime("init") << "\n";
    }

private:
    std::list<InArchive> inArchives;
};
