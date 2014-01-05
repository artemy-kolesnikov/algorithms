#pragma once

#include <cstring>
#include <fstream>

#include <data.h>
#include <exception.h>

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
