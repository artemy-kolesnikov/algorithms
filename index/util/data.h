#pragma once

#include <cstdlib>
#include <cstring>
#include <istream>
#include <ostream>
#include <stdint.h>
#include <vector>

#include <archive.h>

class Key : private std::vector<unsigned char> {
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

    static size_t bytesUsed() {
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

    static size_t bytesUsed() {
        return Key::bytesUsed() + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t);
    }
};
