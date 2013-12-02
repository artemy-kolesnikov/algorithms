#pragma once

#include <cstring>
#include <fstream>

#include <archive.h>
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

    void serialize(OutArchive& out) const {
        key.serialize(out);
        out.write(filePos);
        out.write(canary);
    }

    void deserialize(InArchive& in) {
        key.deserialize(in);
        in.read(filePos);
        in.read(canary);
    }

    size_t bytesUsed() const {
        return (key.bytesUsed() + sizeof(uint64_t) + sizeof(uint64_t));
    }

    Key key;
    uint64_t filePos;
    uint64_t canary;
};
