#pragma once

#include <string>
#include <vector>
#include <fstream>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <archive.h>
#include <exception.h>

template <typename Entry>
class FileWriter : boost::noncopyable {
public:
    typedef Entry EntryType;

    explicit FileWriter(const std::string& fileName, size_t entryCountInBuffer = 1) :
            out(fileName.c_str()),
            outArchive(std::max(entryCountInBuffer, size_t(1)) * EntryType::bytesUsed()),
            writtenCount(0) {}

    ~FileWriter() {
        flush();
    }

    void write(const EntryType& entry) {
        if (outArchive.eof()) {
            out.write(&outArchive.getBuffer().front(), outArchive.getBuffer().size());
            outArchive.clear();
            writtenCount = 0;
        }

        entry.serialize(outArchive);
        ++writtenCount;
    }

    void flush() {
        size_t offset = writtenCount * EntryType::bytesUsed();
        out.write(&outArchive.getBuffer().front(), offset);
        writtenCount = 0;
    }

private:
    std::ofstream out;
    OutArchive outArchive;
    size_t writtenCount;
};

template <typename FileWriter>
class CopyableFileWriter {
public:
    typedef typename FileWriter::EntryType EntryType;

    explicit CopyableFileWriter(const std::string& fileName, size_t countInBuffer = 1) :
            impl(new FileWriter(fileName, countInBuffer)) {}

    void write(const EntryType& entry) {
        impl->write(entry);
    }

    void flush() {
        impl->flush();
    }

private:
    boost::shared_ptr<FileWriter> impl;
};
