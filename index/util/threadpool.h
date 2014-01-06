#pragma once

#include <boost/utility.hpp>

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

class ThreadPool : boost::noncopyable {
    friend class Worker;
public:
    explicit ThreadPool(size_t threadCount);
    ~ThreadPool();

    template <typename Function>
    void schedule(Function function) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskQueue.push_back(std::function<void()>(function));
        }

        workerCondition.notify_one();
    }

    void wait();
    void waitTasksAndExit();
    void stop();

private:
    typedef std::deque< std::function<void()> > TaskQueue;
    typedef std::list<std::thread> Workers;

    std::mutex queueMutex;
    std::condition_variable workerCondition;

    TaskQueue taskQueue;
    Workers workers;
    bool isStop;
};

void Worker::operator()() {
    std::function<void()> task;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(threadPool->queueMutex);

            while (!threadPool->isStop && threadPool->taskQueue.empty()) {
                threadPool->workerCondition.wait(lock);
            }

            if (threadPool->isStop && threadPool->taskQueue.empty()) {
                return;
            }

            task = threadPool->taskQueue.front();
            threadPool->taskQueue.pop_front();
        }

        task();
    }
}

ThreadPool::ThreadPool(size_t threadCount) :
        isStop(false) {
    for (size_t index = 0; index < threadCount; ++index) {
        workers.push_back(std::thread(Worker(this)));
    }
}

ThreadPool::~ThreadPool() {
    if (isStop) {
        return;
    }

    stop();

    wait();
}

void ThreadPool::wait() {
    for (std::thread& thread : workers) {
        thread.join();
    }
}

void ThreadPool::waitTasksAndExit() {
    stop();
    wait();
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        isStop = true;
    }

    workerCondition.notify_all();
}
