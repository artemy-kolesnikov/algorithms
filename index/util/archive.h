#pragma once

#include <cassert>
#include <cstring>
#include <vector>

class OutArchive {
public:
    explicit OutArchive(size_t bufferSize) :
            buffer(std::max(bufferSize, size_t(1)), 0),
            currentPtr(&buffer.front()),
            bufferEnd(&buffer.back() + 1) {}

    template <typename T>
    void write(const T& value) {
        size_t size = sizeof(value);

        assert(currentPtr + size <= bufferEnd);

        memcpy(currentPtr, &value, size);
        currentPtr += size;
    }

    template <typename T>
    void write(const T* ptr, size_t count) {
        size_t size = count * sizeof(T);

        assert(currentPtr + size <= bufferEnd);

        memcpy(currentPtr, ptr, size);
        currentPtr += size;
    }

    const std::vector<char>& getBuffer() const {
        return buffer;
    }

    bool eof() const {
        return currentPtr == bufferEnd;
    }

    void clear() {
        currentPtr = &buffer.front();
    }

private:
    std::vector<char> buffer;
    char* currentPtr;
    char* const bufferEnd;
};

class InArchive {
public:
    InArchive() :
            bufferBegin(0),
            bufferEnd(0),
            currentPtr(0) {}

    InArchive(const char* bufBegin, const char* bufEnd) :
            bufferBegin(bufBegin),
            bufferEnd(bufEnd),
            currentPtr(bufferBegin) {}

    void setBuffer(const char* bufBegin, const char* bufEnd) {
        bufferBegin = bufBegin;
        bufferEnd = bufEnd;
        currentPtr = bufferBegin;
    }

    template <typename T>
    void read(T& value) {
        size_t size = sizeof(value);

        assert(currentPtr + size <= bufferEnd);

        memcpy(&value, currentPtr, size);
        currentPtr += size;
    }

    template <typename T>
    void read(T* ptr, size_t count) {
        size_t size = sizeof(T) * count;

        assert(currentPtr + size <= bufferEnd);

        memcpy(ptr, currentPtr, size);
        currentPtr += size;
    }

    bool eof() const {
        return currentPtr == bufferEnd;
    }

private:
    const char* bufferBegin;
    const char* bufferEnd;
    const char* currentPtr;
};
