#pragma once

#include <serializer.h>

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

namespace _Impl {

const size_t ITEMS_IN_CHUNK = 1 << 20;

struct DummyEventCallback {
    void operator()(SortEventType, int) {}
};

template <typename EntryType, typename EventCallback>
void createChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles, size_t itemsInChunk, EventCallback eventCallback) {
    eventCallback(BeginCreatingChunks, 0);

    size_t count = 0;

    Chunker<EntryType> chunker(chunkDir, itemsInChunk);

    FileInArchive inArchive(dataFileName);

    while (!inArchive.eof()) {
        EntryType data;
        deserialize(data, inArchive);

        if (!isValid(data)) {
            throw Exception() << "Read data is not valid";
        }

        chunker.add(data);

        ++count;
    }

    chunkFiles = chunker.getChunkFileNames();

    eventCallback(DoneCreatingChunks, chunkFiles.size());
}

template <typename EntryType, typename CompareFunc, typename EventCallback>
void sortChunks(const std::list<std::string>& chunkFiles, CompareFunc cmp, EventCallback eventCallback) {
    eventCallback(BeginSortingChunks, 0);

    size_t count = 0;

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, EntryType>(*fileNameIt, cmp);
        eventCallback(ChunkSorted, count++);
    }
}

template <typename EntryType, typename EventCallback>
void mergeChunks(const std::list<std::string>& chunkFiles, const char* outputFileName, EventCallback eventCallback) {
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

}

template <typename EntryType, typename CompareFunc, typename EventCallback>
void externalSortImpl(const char* fileName, const char* chunkDir, const char* outputFileName,
        CompareFunc cmp, size_t itemsInChunk, EventCallback eventCallback) {
    std::list<std::string> chunkFiles;

    _Impl::createChunks<EntryType>(fileName, chunkDir, chunkFiles, itemsInChunk, eventCallback);
    _Impl::sortChunks<EntryType>(chunkFiles, cmp, eventCallback);
    _Impl::mergeChunks<EntryType>(chunkFiles, outputFileName, eventCallback);
}
