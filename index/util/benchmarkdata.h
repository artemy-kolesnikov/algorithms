#pragma once

#include <serializer.h>

#include <array>
#include <cstring>

class BenchmarkData {
public:
    template <typename OutArchive>
    void serialize(OutArchive& out) const {
        out.write(&key.front(), key.size());
        out.write(&payload.front(), payload.size());
    }

    template <typename InArchive>
    void deserialize(InArchive& in) {
        in.read(&key.front(), key.size());
        in.read(&payload.front(), payload.size());
    }

    bool operator < (const BenchmarkData& other) const {
        return (memcmp(&key.front(), &other.key.front(), key.size()) < 0);
    }

    bool isValid() const {
        return true;
    }

private:
    std::array<char, 10> key;
    std::array<char, 90> payload;
};

template <>
struct IsClassSerializable<BenchmarkData> {
    static const bool value = true;
};
