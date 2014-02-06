#include "threadpool.h"

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

            task = std::move(threadPool->taskQueue.front());
            threadPool->taskQueue.pop_front();
        }

        task();

        --threadPool->taskToDoCount;
    }
}

ThreadPool::ThreadPool(size_t threadCount) :
        isStop(false),
        taskToDoCount(0) {
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
