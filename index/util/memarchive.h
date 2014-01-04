#pragma once

#include <cassert>
#include <cstring>
#include <type_traits>
#include <vector>

class MemoryOutArchive {
    enum {
        INITIAL_BUFF_SIZE = 4096
    };

public:
    MemoryOutArchive() :
            buffer(INITIAL_BUFF_SIZE),
            currentPos(0) {}

    template <typename T>
    void write(const T& value, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        write(&value, 1);
    }

    template <typename T>
    void write(const T* ptr, size_t count, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        size_t size = count * sizeof(T);

        if (currentPos + size >= buffer.size()) {
            buffer.resize(buffer.size() * 2);
        }

        memcpy(&buffer[currentPos], ptr, size);
        currentPos += size;
    }

    const std::vector<char>& getBuffer() const {
        return buffer;
    }

    void clear() {
        std::vector<char>().swap(buffer);
    }

    size_t pos() const {
        return currentPos;
    }

private:
    std::vector<char> buffer;
    size_t currentPos;
};

class MemoryInArchive {
public:
    MemoryInArchive() :
            bufferBegin(0),
            bufferEnd(0),
            currentPtr(0) {}

    MemoryInArchive(const char* bufBegin, const char* bufEnd) :
            bufferBegin(bufBegin),
            bufferEnd(bufEnd),
            currentPtr(bufferBegin) {}

    void setBuffer(const char* bufBegin, const char* bufEnd) {
        bufferBegin = bufBegin;
        bufferEnd = bufEnd;
        currentPtr = bufferBegin;
    }

    template <typename T>
    bool read(T& value, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        return read(&value, 1);
    }

    template <typename T>
    bool read(T* ptr, size_t count, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        if (eof()) {
            return false;
        }

        size_t size = sizeof(T) * count;

        if (currentPtr + size > bufferEnd) {
            throw Exception() << "Can't read" << size << "bytes from memory because it is out of bounds";
        }

        memcpy(ptr, currentPtr, size);
        currentPtr += size;

        return true;
    }

    bool eof() const {
        return currentPtr == bufferEnd;
    }

    size_t pos() const {
        return currentPtr - bufferBegin;
    }

    void skip(size_t bytes) {
        if (currentPtr + bytes > bufferEnd) {
            throw Exception() << "Can't skip" << bytes << "bytes because it is out of bounds";
        }

        currentPtr += bytes;
    }

private:
    const char* bufferBegin;
    const char* bufferEnd;
    const char* currentPtr;
};
