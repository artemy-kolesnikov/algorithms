#pragma once

#include <noncopyable.h>

#include <atomic>
#include <condition_variable>
#include <deque>
#include <list>
#include <mutex>
#include <thread>
#include <functional>

class ThreadPool;

class Worker {
public:
    Worker(ThreadPool* pool) :
            threadPool(pool) {}

    void operator()();

private:
    ThreadPool* threadPool;
};

class ThreadPool : Noncopyable {
    friend class Worker;
public:
    explicit ThreadPool(size_t threadCount);
    ~ThreadPool();

    template <typename Function>
    void schedule(Function&& function) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskQueue.push_back(std::function<void()>(std::move(function)));
        }

        workerCondition.notify_one();

        ++taskToDoCount;
    }

    void wait();
    void waitTasksAndExit();
    void stop();

    size_t tasksToDo() const {
        return taskToDoCount;
    }

private:
    typedef std::deque< std::function<void()> > TaskQueue;
    typedef std::list<std::thread> Workers;

    std::mutex queueMutex;
    std::condition_variable workerCondition;

    TaskQueue taskQueue;
    Workers workers;
    bool isStop;
    std::atomic_int taskToDoCount;
};
