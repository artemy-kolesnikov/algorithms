#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <archive.h>
#include <exception.h>

template <typename Entry>
class FileReader {
public:
    typedef Entry EntryType;

    explicit FileReader(const std::string& fileName, size_t countInBuffer = 1) :
            in(fileName.c_str()),
            buffer(std::max(countInBuffer, size_t(1)) * EntryType::bytesUsed(), 0) {
        if (!in.is_open()) {
            throw Exception() << "Can't open file" << fileName;
        }

        in.read(&buffer.front(), buffer.size());
        inArchive.setBuffer(&buffer.front(), &buffer.front() + in.gcount());
    }

    bool read(EntryType& entry) {
        if (inArchive.eof()) {
            in.read(&buffer.front(), buffer.size());
            inArchive.setBuffer(&buffer.front(), &buffer.front() + in.gcount());
        }

        if (inArchive.eof()) {
            return false;
        }

        entry.deserialize(inArchive);

        return true;
    }

    EntryType* read() {
        if (!read(lastRead)) {
            return 0;
        }

        return &lastRead;
    }

private:
    FileReader(const FileReader& other);
    FileReader& operator = (const FileReader& other);

private:
    std::ifstream in;
    std::vector<char> buffer;
    InArchive inArchive;
    EntryType lastRead;
};

template <typename FileReader>
class CopyableFileReader {
public:
    typedef typename FileReader::EntryType EntryType;

    explicit CopyableFileReader(const std::string& fileName, size_t countInBuffer = 1) :
            impl(new FileReader(fileName, countInBuffer)) {}

    bool read(EntryType& entry) {
        return impl->read(entry);
    }

    const EntryType* read() {
        return impl->read();
    }

private:
    boost::shared_ptr<FileReader> impl;
};
