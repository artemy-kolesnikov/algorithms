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

    explicit FileWriter(const std::string& fileName) :
            out(fileName.c_str()) {}

    void write(const EntryType& entry) {
        OutArchive outArchive(entry.bytesUsed());
        entry.serialize(outArchive);
        out.write(&outArchive.getBuffer().front(), outArchive.getBuffer().size());
    }

private:
    std::ofstream out;
};

template <typename FileWriter>
class CopyableFileWriter {
public:
    typedef typename FileWriter::EntryType EntryType;

    explicit CopyableFileWriter(const std::string& fileName) :
            impl(new FileWriter(fileName)) {}

    void write(const EntryType& entry) {
        impl->write(entry);
    }

    void flush() {
        impl->flush();
    }

private:
    boost::shared_ptr<FileWriter> impl;
};
