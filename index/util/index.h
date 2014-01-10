#pragma once

#include <serializer.h>

#include <cstring>
#include <fstream>

#include <data.h>
#include <exception.h>
#include <sorter.h>

namespace _Impl {

template <typename DataEntry, typename IndexEntry, typename CreateKeyFunc>
void createIndexChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles,
        size_t itemsInChunk, CreateKeyFunc createKeyFunc) {
    Chunker<IndexEntry> chunker(chunkDir, itemsInChunk);

    FileInArchive inArchive(dataFileName);

    while (!inArchive.eof()) {
        DataEntry data;
        deserialize(data, inArchive);

        if (!isValid(data)) {
            throw Exception() << "Read data is not valid";
        }

        chunker.add(createKeyFunc(data, inArchive.pos()));
    }

    chunkFiles = chunker.getChunkFileNames();
}

}

struct IndexEntry {
    IndexEntry() :
            filePos(0),
            canary(DataHeader::CANARY) {}

    IndexEntry(Key k, uint64_t p) :
            key(k),
            filePos(p),
            canary(DataHeader::CANARY) {}

    bool operator < (const IndexEntry& other) const {
        return key < other.key;
    }

    template <typename OutArchive>
    void serialize(OutArchive& out) const {
        key.serialize(out);
        out.write(filePos);
        out.write(canary);
    }

    template <typename InArchive>
    void deserialize(InArchive& in) {
        key.deserialize(in);
        in.read(filePos);
        in.read(canary);
    }
 
    bool isValid() const {
        return canary == DataHeader::CANARY;
    }

    Key key;
    uint64_t filePos;
    uint64_t canary;
};

template <>
struct IsClassSerializable<IndexEntry> {
    static const bool value = true;
};

template <typename DataEntry, typename IndexEntry, typename CreateKeyFunc>
void createIndex(const char* dataFileName, const char* chunkDir, const char* outputFileName,
        size_t itemsInChunk, size_t threadCount, CreateKeyFunc createKeyFunc) {
    std::list<std::string> chunkFiles;

    _Impl::createIndexChunks<DataEntry, IndexEntry>(dataFileName, chunkDir, chunkFiles, itemsInChunk, createKeyFunc);
    sortChunks<IndexEntry>(chunkFiles, threadCount);
    mergeChunks<IndexEntry>(chunkFiles, outputFileName);
}

