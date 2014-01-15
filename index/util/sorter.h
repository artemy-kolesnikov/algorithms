#pragma once

#include <chunker.h>
#include <merger.h>
#include <serializer.h>
#include <threadpool.h>

#include <algorithm>
#include <functional>
#include <list>
#include <vector>

enum SortEventType {
    BeginCreatingChunks,
    DoneCreatingChunks,
    BeginSortingChunks,
    ChunkSorted,
    EndSortingChunks,
    BeginMergingChunks,
    DoneMergingChunks
};

namespace _Impl {

struct DefaultEventCallback {
    void operator()(SortEventType, int) {}
};

struct DefaultChunkerFunction {
    template <typename Entry, typename Chunker, typename InArchive>
    void operator()(const Entry& entry, Chunker& chunker, InArchive&) {
        chunker.add(entry);
    }
};

struct DefaultSortFunction {
    template <typename RandomAccessIterator>
    void operator()(RandomAccessIterator begin, RandomAccessIterator end) {
        std::sort(begin, end);
    }
};

template <typename InArchive, typename OutArchive, typename EntryType, typename SortFunction>
void sortFileInMemory(const std::string& fileName, SortFunction sort) {
    InArchive inArchive(fileName);

    std::vector<EntryType> dataVector;

    while (!inArchive.eof()) {
        EntryType entry;
        deserialize(entry, inArchive);

        dataVector.push_back(entry);
    }

    //std::sort(dataVector.begin(), dataVector.end(), cmp);
    sort(dataVector.begin(), dataVector.end());

    OutArchive outArchive(fileName);
    std::for_each(dataVector.begin(), dataVector.end(), [&outArchive](const EntryType& entry) {
        serialize(entry, outArchive);
    });
}

}

template <typename EntryType, typename ChunkerFunction = _Impl::DefaultChunkerFunction, typename EventCallback = _Impl::DefaultEventCallback>
void createChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles,
        size_t itemsInChunk, ChunkerFunction chunkerFunction = _Impl::DefaultChunkerFunction(), EventCallback eventCallback = _Impl::DefaultEventCallback()) {
    eventCallback(BeginCreatingChunks, 0);

    Chunker<EntryType> chunker(chunkDir, itemsInChunk);

    FileInArchive inArchive(dataFileName);

    while (!inArchive.eof()) {
        EntryType data;
        deserialize(data, inArchive);

        if (!isValid(data)) {
            throw Exception() << "Read data is not valid";
        }

        chunkerFunction(data, chunker, inArchive);
    }

    chunkFiles = chunker.getChunkFileNames();

    eventCallback(DoneCreatingChunks, chunkFiles.size());
}


template <typename EntryType, typename Chunker = Chunker<EntryType>, typename ChunkerFunction = _Impl::DefaultChunkerFunction,
        typename SortFunction = _Impl::DefaultSortFunction, typename EventCallback = _Impl::DefaultEventCallback>
void createAndSortChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles,
        size_t itemsInChunk, size_t threadCount, ChunkerFunction chunkerFunction = _Impl::DefaultChunkerFunction(),
        SortFunction sort = SortFunction(), EventCallback eventCallback = _Impl::DefaultEventCallback()) {
    eventCallback(BeginCreatingChunks, 0);

    ThreadPool threadPool(threadCount);

    Chunker chunker(chunkDir, itemsInChunk,
            [&](const char* chunkFileName) -> void {
                threadPool.schedule([=]() {
                    _Impl::sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, typename Chunker::EntryType>(chunkFileName, sort);
                });
            }
    );

    FileInArchive inArchive(dataFileName);

    while (!inArchive.eof()) {
        EntryType data;
        deserialize(data, inArchive);

        if (!isValid(data)) {
            throw Exception() << "Read data is not valid";
        }

        chunkerFunction(data, chunker, inArchive);
    }

    chunkFiles = chunker.getChunkFileNames();

    chunker.flush();

    std::string chunkFileName = chunkFiles.back();
    threadPool.schedule([&chunkFileName, &sort]() {
        _Impl::sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, typename Chunker::EntryType>(chunkFileName, sort);
    });

    threadPool.waitTasksAndExit();

    eventCallback(DoneCreatingChunks, chunkFiles.size());
}

template <typename EntryType, typename SortFunction = _Impl::DefaultSortFunction,
        typename EventCallback = _Impl::DefaultEventCallback>
void sortChunks(const std::list<std::string>& chunkFiles, size_t threadCount,
        SortFunction sort = _Impl::DefaultSortFunction(), EventCallback eventCallback = _Impl::DefaultEventCallback()) {
    eventCallback(BeginSortingChunks, 0);

    ThreadPool threadPool(threadCount);

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        threadPool.schedule([=]() {
            _Impl::sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, EntryType>(*fileNameIt, sort);
        });
    }

    threadPool.waitTasksAndExit();
}

template <typename EntryType, typename EventCallback = _Impl::DefaultEventCallback>
void mergeChunks(const std::list<std::string>& chunkFiles, const char* outputFileName, EventCallback eventCallback = _Impl::DefaultEventCallback()) {
    eventCallback(BeginMergingChunks, 0);

    std::list<CopyableFileInArchive> archives;

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        archives.push_back(CopyableFileInArchive(*fileNameIt));
    }

    Merger<EntryType, CopyableFileInArchive> merger(archives);
    FileOutArchive outArchive(outputFileName);
    merger.merge([&outArchive](const EntryType& entry) -> void {
        serialize(entry, outArchive);
    });

    eventCallback(DoneMergingChunks, 0);
}

template <typename EntryType, typename SortFunction = _Impl::DefaultSortFunction, typename EventCallback = _Impl::DefaultEventCallback>
void externalSort(const char* fileName, const char* chunkDir, const char* outputFileName,
        size_t itemsInChunk, size_t threadCount, SortFunction sort = _Impl::DefaultSortFunction(),
        EventCallback eventCallback = _Impl::DefaultEventCallback()) {
    std::list<std::string> chunkFiles;

    createAndSortChunks<EntryType>(fileName, chunkDir, chunkFiles, itemsInChunk, threadCount, _Impl::DefaultChunkerFunction(), sort, eventCallback);
    //mergeChunks<EntryType>(chunkFiles, outputFileName, eventCallback);
}
