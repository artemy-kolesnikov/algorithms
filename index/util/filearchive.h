#pragma once

#include <exception.h>
#include <memarchive.h>
#include <mmapper.h>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <fstream>
#include <string>
#include <type_traits>

class FileOutArchive : boost::noncopyable {
public:
    explicit FileOutArchive(const std::string& fileName) :
            out(fileName.c_str()) {}

    template <typename T>
    void write(const T& value, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        out.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    template <typename T>
    void write(const T* ptr, size_t count, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        out.write(reinterpret_cast<const char*>(ptr), sizeof(T) * count);
    }

    size_t pos() {
        return out.tellp();
    }

private:
    std::ofstream out;
};

class CopyableFileOutArchive {
public:
    explicit CopyableFileOutArchive(const std::string& fileName) :
            impl(new FileOutArchive(fileName)) {}

    template <typename T>
    void write(const T& value) {
        impl->write(value);
    }

    template <typename T>
    void write(const T* ptr, size_t count) {
        impl->write(ptr, count);
    }

    size_t pos() const {
        return impl->pos();
    }

private:
    boost::shared_ptr<FileOutArchive> impl;
};

class FileInArchive : boost::noncopyable {
public:
    explicit FileInArchive(const std::string& fName) :
            fileName(fName),
            mMapper(fName) {
        mMapper.map();
        const char* beginPtr = mMapper.getBeginPtr();
        const char* endPtr = mMapper.getEndPtr();
        memArchive.setBuffer(beginPtr, endPtr);
    }

    template <typename T>
    bool read(T& entry, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        return read(&entry, 1);
    }

    template <typename T>
    bool read(T* ptr, size_t count, typename std::enable_if<std::is_pod<T>::value>::type * = 0) {
        size_t bytesToRead = count * sizeof(T);
        return memArchive.read(reinterpret_cast<char*>(ptr), bytesToRead);
    }

    bool eof() {
        return memArchive.eof();
    }

    size_t pos() {
        return memArchive.pos();
    }

    void skip(size_t bytes) {
        return memArchive.skip(bytes);
    }

private:
    MemoryInArchive memArchive;
    ReadOnlyMemMapper mMapper;
    std::string fileName;
};

class CopyableFileInArchive {
public:
    explicit CopyableFileInArchive(const std::string& fName) :
            impl(new FileInArchive(fName)) {}

    template <typename T>
    bool read(T& entry) {
        return impl->read(entry);
    }

    template <typename T>
    bool read(T* ptr, size_t count) {
        return impl->read(ptr, count);
    }

    bool eof() {
        return impl->eof();
    }

    size_t pos() const {
        return impl->pos();
    }

    void skip(size_t bytes) {

    }

private:
    boost::shared_ptr<FileInArchive> impl;
};
