#pragma once

#include <cerrno>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <boost/utility.hpp>

class ReadOnlyMemMapper : boost::noncopyable {
public:
    explicit ReadOnlyMemMapper(const std::string& fName) :
        fileName(fName),
        fileFd(0),
        beginPtr(0),
        endPtr(0),
        maped(false),
        mapSize(0) {}

    ~ReadOnlyMemMapper() {
        unmap();
    }

    const char* getBeginPtr() const {
        return beginPtr;
    }

    const char* getEndPtr() const {
        return endPtr;
    }

    void map() {
        if (!maped) {
            struct stat fileStat;
            stat(fileName.c_str(), &fileStat);

            mapSize = fileStat.st_size;

            fileFd = open(fileName.c_str(), O_RDONLY);

            if (fileFd == -1) {
                throw Exception() << "Can't open file" << fileName << "for mapping" << strerror(errno);
            }

            beginPtr = reinterpret_cast<char*>(::mmap(0, mapSize, PROT_READ, MAP_SHARED, fileFd, 0));
            endPtr = beginPtr + mapSize;

            if (!beginPtr) {
                close(fileFd);
                throw Exception() << "Can't map file" << fileName << strerror(errno);
            }

            maped = true;
        }
    }

    void unmap() {
        if (maped) {
            ::munmap(beginPtr, mapSize);
            close(fileFd);

            beginPtr = 0;
            endPtr = 0;
        }
    }

private:
    std::string fileName;
    int fileFd;
    char* beginPtr;
    char* endPtr;
    bool maped;
    size_t mapSize;
};
