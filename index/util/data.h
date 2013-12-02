#pragma once

#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>
#include <stdint.h>
#include <vector>

#include <archive.h>

class Key : std::vector<unsigned char> {
public:
    enum {
        SIZE = 64,
    };

    Key() : std::vector<unsigned char>(SIZE, 0) {}  

    using std::vector<unsigned char>::front;
    using std::vector<unsigned char>::operator=;
    using std::vector<unsigned char>::operator[];
    using std::vector<unsigned char>::size;
    using std::vector<unsigned char>::value_type;

    bool operator < (const Key& other) const {
        return (memcmp(&front(), &other.front(), sizeof(unsigned char) * size()) < 0);
    }

    bool operator == (const Key& other) const {
        return (memcmp(&front(), &other.front(), sizeof(unsigned char) * size()) == 0);
    }

    bool operator > (const Key& other) const {
        return (memcmp(&front(), &other.front(), sizeof(unsigned char) * size()) > 0);
    }

    void serialize(OutArchive& out) const {
        out.write(&front(), size());
    }

    void deserialize(InArchive& in) {
        in.read(&front(), size());
    }

    size_t bytesUsed() const {
        return sizeof(Key::value_type) * Key::SIZE;
    }
};

class DataHeader {
public:
    enum {
        CANARY = 0xABCDEF
    };

    DataHeader() :
            flags(0),
            canary(CANARY),
            size(0) { }

    DataHeader(uint64_t flg, uint64_t sz) : 
            flags(flg),
            canary(CANARY),
            size(sz) { }

    Key             key;
    uint64_t        flags;
    uint64_t        canary;
    uint64_t        size;

    void serialize(OutArchive& out) const {
        key.serialize(out);

        out.write(flags);
        out.write(canary);
        out.write(size);
    }

    void deserialize(InArchive& in) {
        key.deserialize(in);
        in.read(flags);
        in.read(canary);
        in.read(size);
    }

    size_t bytesUsed() const {
        return key.bytesUsed() + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t);
    }
};

class DataEntry {
public:
    DataEntry() {}

    DataEntry(const DataHeader& header_, const std::vector<char>& data_) :
            header(header_),
            data(data_) {}

    void serialize(OutArchive& out) const {
        header.serialize(out);
        if (!data.empty()) {
            out.write(&data.front(), data.size());
        }
    }

    void deserialize(InArchive& in) {
        header.deserialize(in);
        if (header.size) {
            data.resize(header.size);
            in.read(&data.front(), data.size());
        }
    }

    size_t bytesUsed() const {
        return header.bytesUsed() + data.size();
    }

    bool operator < (const DataEntry& other) const {
        return header.key < other.header.key;
    }

private:
    DataHeader header;
    std::vector<char> data;
};
