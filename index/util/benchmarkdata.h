#pragma once

#include <serializer.h>

#include <array>
#include <cstring>

template <typename T, size_t Number>
class Array {
public:
    Array() {
        data = new T[Number];
    }

    ~Array() {
        delete [] data;
    }

    Array(const Array& other) {
        data = new T[Number];
        std::copy(other.begin(), other.end(), begin());
        //std::cout << "Copy ctor\n";
    }

    Array& operator = (const Array& other) {
        std::copy(other.begin(), other.end(), begin());
        //std::cout << "Copy asignment\n";
        return *this;
    }

    Array(Array&& other) {
        std::swap(data, other.data);
        other.data = nullptr;
        //std::cout << "Move ctor\n";
    }

    Array& operator=(Array&& other) {
        std::swap(data, other.data);
        other.data = nullptr;
        //std::cout << "Move asignment\n";
        return *this;
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

    T& operator[](size_t index) {
        return data[index];
    }

    bool operator == (const Array& other) const {
        return (memcmp(data, other.data, Number * sizeof(T)) == 0);
    }

    size_t size() const {
        return Number;
    }

    const T* begin() const {
        return data;
    }

    T* begin() {
        return data;
    }

    const T* end() const {
        return data + Number;
    }

    T* end() {
        return data + Number;
    }

    const T& front() const {
        return data[0];
    }

    T& front() {
        return data[0];
    }

    const T& back() const {
        return data[Number - 1];
    }

    T& back() {
        return data[Number - 1];
    }

    bool operator < (const Array& other) const {
        return (memcmp(data, other.data, Number * sizeof(T)) < 0);
    }

private:
    T* data;
};

class BenchmarkData {
public:
    BenchmarkData() /*:
        isMin(false),
        isMax(false)*/ {}
    BenchmarkData(const BenchmarkData& other) = default;
    BenchmarkData& operator = (const BenchmarkData& other) = default;

    BenchmarkData(BenchmarkData&& other) {
        key = std::move(other.key);
        payload = std::move(other.payload);
    }

    BenchmarkData& operator=(BenchmarkData&& other) {
        key = std::move(other.key);
        payload = std::move(other.payload);
        return *this;
    }

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
        /*if (isMin && other.isMin) {
            return false;
        }

        if (isMax && other.isMax) {
            return false;
        }

        if (isMin) {
            return true;
        }

        if (isMax) {
            return false;
        }*/

        return key < other.key;
    }

    /*void setMin() {
        isMin = true;
        isMax = false;
    }

    void setMax() {
        isMax = true;
        isMin = false;
    }*/

    bool operator == (const BenchmarkData& other) const {
        return key == other.key;
    }

    bool isValid() const {
        return true;
    }

    Array<char, 10> key;
    Array<char, 90> payload;
    /*std::array<char, 10> key;
    std::array<char, 90> payload;
    bool isMin;
    bool isMax;*/
};

template <>
struct IsClassSerializable<BenchmarkData> {
    static const bool value = true;
};
