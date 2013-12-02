#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <archive.h>
#include <exception.h>

#include <iostream>

template <typename Entry>
class FileReader : boost::noncopyable {
public:
    typedef Entry EntryType;

    explicit FileReader(const std::string& fName) :
            in(fName.c_str()), fileName(fName) {
        if (!in.is_open()) {
            throw Exception() << "Can't open file" << fName;
        }
    }

    bool read(EntryType& entry) {
        std::vector<char> buffer(entry.bytesUsed(), 0);
        in.read(&buffer.front(), buffer.size());

        if (in.gcount() == 0) {
            return false;
        }

        if (in.gcount() < buffer.size()) {
            throw Exception() << "Can't read" << buffer.size() << "bytes from file" << fileName;
        }

        InArchive inArchive;
        inArchive.setBuffer(&buffer.front(), &buffer.front() + buffer.size());

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
    std::ifstream in;
    EntryType lastRead;
    std::string fileName;
};

template <typename FileReader>
class CopyableFileReader {
public:
    typedef typename FileReader::EntryType EntryType;

    explicit CopyableFileReader(const std::string& fileName) :
            impl(new FileReader(fileName)) {}

    bool read(EntryType& entry) {
        return impl->read(entry);
    }

    const EntryType* read() {
        return impl->read();
    }

private:
    boost::shared_ptr<FileReader> impl;
};
