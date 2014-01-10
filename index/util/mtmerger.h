#pragma once

#include <filearchive.h>
#include <threadpool.h>

#include <condition_variable>
#include <list>
#include <mutex>
#include <queue>
#include <sstream>

template <typename ItemType, typename InArchive>
class MtMerger {
    typedef std::queue<InArchive> MergeQueue;
    typedef std::list<InArchive> ArchiveList;

public:
    MtMerger(const std::list<InArchive>& archives, size_t thrdCnt, size_t kMergeCnt, const char* tmpDir) :
            inArchives(archives),
            threadCount(thrdCnt),
            kMergeCount(kMergeCnt),
            tempDir(tmpDir),
            chunkCounter(0) {
        for (const InArchive& arch : archives) {
            mergeQueue.push(arch);
        }
    }

    template <typename ProcessFunction>
    void merge(ProcessFunction process) {
        ThreadPool threadPool(threadCount);

        bool lastTask = false;

        while (!lastTask) {
            ArchiveList arcList;
            size_t addedArch = 0;

            {
                std::unique_lock<std::mutex> lock(queueMutex);
                while (addedArch < kMergeCount) {
                    while (mergeQueue.empty() && threadPool.tasksToDo() > 0) {
                        condition.wait(lock);
                    }

                    if (mergeQueue.empty()) {
                        break;
                    }

                    arcList.push_back(mergeQueue.front());
                    mergeQueue.pop();
                    ++addedArch;
                }
            }

            std::string chunkFileName = getNextChunkFileName();

            if (mergeQueue.empty() && threadPool.tasksToDo() == 0) {
                lastTask = true;
                threadPool.schedule([=]() {
                    Merger<ItemType, InArchive> merger(arcList);
                    merger.merge(process);
                });
            } else {
                threadPool.schedule([=]() {
                    TwoWayMerger<ItemType, InArchive> merger(arcList.front(), arcList.back());
                    //Merger<ItemType, InArchive> merger(arcList);
                    FileOutArchive outArchive(chunkFileName);
                    merger.merge([&outArchive](const ItemType& entry) -> void {
                        serialize(entry, outArchive);
                    });

                    outArchive.flush();


                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        mergeQueue.push(CopyableFileInArchive(chunkFileName));
                    }

                    condition.notify_all();
                });
            }
        }

        threadPool.waitTasksAndExit();
    }

private:
    std::string getNextChunkFileName() const {
        std::stringstream sstr;
        sstr << tempDir << "/mtmerge_chunk_" << chunkCounter++ << ".dat";
        return sstr.str();
    }

private:
    std::list<InArchive> inArchives;
    MergeQueue mergeQueue;
    std::mutex queueMutex;
    std::condition_variable condition;
    size_t threadCount;
    size_t kMergeCount;
    std::string tempDir;
    mutable size_t chunkCounter;
};
