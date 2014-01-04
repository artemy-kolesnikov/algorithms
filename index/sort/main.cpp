#include <cassert>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <sys/time.h>

#include <chunker.h>
#include <data.h>
#include <filearchive.h>
#include <memarchive.h>
#include <merger.h>
#include <mmapper.h>
#include <sorter.h>

namespace {

void printUsage() {
    std::cout << "Usage: sort_file data_file_name tmp_data_dir out_file_name\n";
}

typedef Merger<DataEntry, CopyableFileInArchive, CopyableFileOutArchive> DataMerger;

typedef Chunker<DataEntry> DataChunker;

void createChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles, size_t itemsInChunk) {
    std::cout << "Creating chunks...\n";

    size_t count = 0;

    DataChunker chunker(chunkDir, itemsInChunk);

    ReadOnlyMemMapper mmapper(dataFileName);

    mmapper.map();

    const char* beginPtr = mmapper.getBeginPtr();
    const char* endPtr = mmapper.getEndPtr();

    MemoryInArchive inArchive(beginPtr, endPtr);

    while (!inArchive.eof()) {
        DataEntry data;
        data.deserialize(inArchive);

        assert(data.header.canary == DataHeader::CANARY);

        chunker.add(data);

        ++count;
    }

    std::cout << "Done\n";

    std::cout << count << " data items" << "\n";

    chunkFiles = chunker.getChunkFileNames();
}

void sortChunks(const std::list<std::string>& chunkFiles) {
    std::cout << "Sorting chunks\n";

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, DataEntry>(*fileNameIt);

        std::cout << "Chunk " << *fileNameIt << " sorted\n";
    }
}

void mergeChunks(const std::list<std::string>& chunkFiles, const char* outputFileName) {
    std::cout << "Merging chunks...\n";

    std::list<CopyableFileInArchive> archives;

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        archives.push_back(CopyableFileInArchive(*fileNameIt));
    }

    CopyableFileOutArchive outArchive(outputFileName);
    DataMerger merger(archives, outArchive);
    merger.merge();

    std::cout << "Done\n";
}

void sortData(const char* dataFileName, const char* chunkDir, const char* outputFileName) {
    std::list<std::string> chunkFiles;

    const size_t ITEMS_IN_CHUNK = (1 << 20);

    createChunks(dataFileName, chunkDir, chunkFiles, ITEMS_IN_CHUNK);
    sortChunks(chunkFiles);
    mergeChunks(chunkFiles, outputFileName);
}

}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }

    const char* dataFileName = argv[1];
    const char* chunkDir = argv[2];
    const char* outputFileName = argv[3];

    try {
        sortData(dataFileName, chunkDir, outputFileName);
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
