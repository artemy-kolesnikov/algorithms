#pragma once

#include <list>
#include <sstream>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

#include <filewriter.h>

template <typename EntryType>
class Chunker : boost::noncopyable {
public:
    Chunker(const std::string& chnkDir, size_t countInChnk) :
            chunkDir(chnkDir),
            countInChunk(countInChnk),
            chunkDataCounter(0),
            chunkCounter(0) {
        std::string chunkFileName = getChunkFileName();
        chunkFileNames.push_back(chunkFileName);
        fileWriter.reset(new FileWriter<EntryType>(chunkFileName));
    }

    void add(const EntryType& entry) {
        ++chunkDataCounter;

        fileWriter->write(entry);

        if (chunkDataCounter == countInChunk) {
            ++chunkCounter;
            std::string chunkFileName = getChunkFileName();
            chunkFileNames.push_back(chunkFileName);
            fileWriter.reset(new FileWriter<EntryType>(chunkFileName));
            chunkDataCounter = 0;
        }
    }

    const std::list<std::string> getChunkFileNames() const {
        return chunkFileNames;
    }

private:
    std::string getChunkFileName() const {
        std::stringstream sstr;
        sstr << chunkDir << "/chunk_" << chunkCounter << ".dat";
        return sstr.str();
    }

private:
    const std::string chunkDir;
    const size_t countInChunk;
    size_t chunkDataCounter;
    size_t chunkCounter;
    std::list<std::string> chunkFileNames;
    boost::shared_ptr< FileWriter<EntryType> > fileWriter;
};
