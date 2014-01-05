#pragma once

#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>
#include <stdint.h>
#include <vector>

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
        return (memcmp(&front(), &other.front(), sizeof(unsigned char) * SIZE) < 0);
    }

    bool operator == (const Key& other) const {
        return (memcmp(&front(), &other.front(), sizeof(unsigned char) * SIZE) == 0);
    }

    bool operator > (const Key& other) const {
        return (memcmp(&front(), &other.front(), sizeof(unsigned char) * SIZE) > 0);
    }

    template <typename OutArchive>
    void serialize(OutArchive& out) const {
        out.write(&front(), SIZE);
    }

    template <typename InArchive>
    void deserialize(InArchive& in) {
        in.read(&front(), SIZE);
    }
};

class DataHeader {
public:
    enum {
        CANARY = 0xABCDEF
    };

    DataHeader() :
            canary(CANARY),
            dataSize(0) { }

    DataHeader(uint64_t flg, uint64_t sz) : 
            canary(CANARY),
            dataSize(sz) { }

    Key             key;
    uint64_t        canary;
    uint64_t        dataSize;

    template <typename OutArchive>
    void serialize(OutArchive& out) const {
        key.serialize(out);
        out.write(canary);
        out.write(dataSize);
    }

    template <typename InArchive>
    void deserialize(InArchive& in) {
        key.deserialize(in);
        in.read(canary);
        in.read(dataSize);
    }

    bool isValid() const {
        return canary == DataHeader::CANARY;
    }
};

class DataEntry {
public:
    DataEntry() {}

    DataEntry(const DataHeader& header_, const std::vector<char>& data_) :
            header(header_),
            data(data_) {}

    template <typename OutArchive>
    void serialize(OutArchive& out) const {
        header.serialize(out);
        if (!data.empty()) {
            out.write(&data.front(), data.size());
        }
    }

    template <typename InArchive>
    void deserialize(InArchive& in) {
        header.deserialize(in);
        data.clear();
        if (header.dataSize) {
            data.resize(header.dataSize);
            in.read(&data.front(), data.size());
        }
    }

    bool operator < (const DataEntry& other) const {
        return header.key < other.header.key;
    }

    bool isValid() const {
        return header.canary == DataHeader::CANARY;
    }

    DataHeader header;
    std::vector<char> data;
};

template <typename T>
struct IsClassSerializable {
    static const bool value = false;
};

template <>
struct IsClassSerializable<Key> {
    static const bool value = true;
};

template <>
struct IsClassSerializable<DataHeader> {
    static const bool value = true;
};

template <>
struct IsClassSerializable<DataEntry> {
    static const bool value = true;
};
