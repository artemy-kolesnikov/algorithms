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

const size_t ITEMS_IN_CHUNK = 1 << 20;

struct DummyEventCallback {
    void operator()(SortEventType, int) {}
};

struct DefaultChunkerFunction {
    template <typename Entry, typename Chunker, typename InArchive>
    void operator()(const Entry& entry, Chunker& chunker, InArchive&) {
        chunker.add(entry);
    }
};

template <typename InArchive, typename OutArchive, typename EntryType, typename CompareFunc = std::less<EntryType> >
void sortFileInMemory(const std::string& fileName, CompareFunc cmp = std::less<EntryType>()) {
    InArchive inArchive(fileName);

    std::vector<EntryType> dataVector;

    while (!inArchive.eof()) {
        EntryType entry;
        deserialize(entry, inArchive);

        dataVector.push_back(entry);
    }

    std::sort(dataVector.begin(), dataVector.end(), cmp);

    OutArchive outArchive(fileName);
    std::for_each(dataVector.begin(), dataVector.end(), [&outArchive](const EntryType& entry) {
        serialize(entry, outArchive);
    });
}

}

template <typename EntryType, typename ChunkerFunction = _Impl::DefaultChunkerFunction, typename EventCallback = _Impl::DummyEventCallback>
void createChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles,
        size_t itemsInChunk, ChunkerFunction chunkerFunction = _Impl::DefaultChunkerFunction(), EventCallback eventCallback = _Impl::DummyEventCallback()) {
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


template <typename EntryType, typename ChunkerFunction = _Impl::DefaultChunkerFunction, typename EventCallback = _Impl::DummyEventCallback>
void createAndSortChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles,
        size_t itemsInChunk, ChunkerFunction chunkerFunction = _Impl::DefaultChunkerFunction(), EventCallback eventCallback = _Impl::DummyEventCallback()) {
    eventCallback(BeginCreatingChunks, 0);

    ThreadPool threadPool(4);

    Chunker<EntryType> chunker(chunkDir, itemsInChunk,
            [&](const char* chunkFileName) -> void {
                threadPool.schedule([=]() {
                    _Impl::sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, EntryType>(chunkFileName);
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
    threadPool.schedule([&chunkFileName]() {
        _Impl::sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, EntryType>(chunkFileName);
    });

    threadPool.waitTasksAndExit();

    eventCallback(DoneCreatingChunks, chunkFiles.size());
}

template <typename EntryType, typename CompareFunc = std::less<EntryType>, typename EventCallback = _Impl::DummyEventCallback>
void sortChunks(const std::list<std::string>& chunkFiles, CompareFunc cmp = std::less<EntryType>(), EventCallback eventCallback = _Impl::DummyEventCallback()) {
    eventCallback(BeginSortingChunks, 0);

    ThreadPool threadPool(4);

    size_t count = 0;

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        threadPool.schedule([=]() {
            _Impl::sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, EntryType>(*fileNameIt, cmp);
        });
    }

    threadPool.waitTasksAndExit();
}

template <typename EntryType, typename EventCallback = _Impl::DummyEventCallback>
void mergeChunks(const std::list<std::string>& chunkFiles, const char* outputFileName, EventCallback eventCallback = _Impl::DummyEventCallback()) {
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

template <typename EntryType, typename CompareFunc, typename EventCallback>
void externalSort(const char* fileName, const char* chunkDir, const char* outputFileName,
        CompareFunc cmp, size_t itemsInChunk, EventCallback eventCallback) {
    std::list<std::string> chunkFiles;

    createAndSortChunks<EntryType>(fileName, chunkDir, chunkFiles, itemsInChunk, _Impl::DefaultChunkerFunction(), eventCallback);
    mergeChunks<EntryType>(chunkFiles, outputFileName, eventCallback);
}
