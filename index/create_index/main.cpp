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
#include <index.h>
#include <merger.h>
#include <sorter.h>

namespace {

void printUsage() {
    std::cout << "Usage: create_index data_file_name chunk_dir out_file_name\n";
}

typedef Merger<IndexEntry, CopyableFileInArchive> IndexMerger;

typedef Chunker<IndexEntry> IndexChunker;

void createChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles, size_t itemsInChunk) {
    std::cout << "Creating chunks...\n";

    size_t count = 0;

    IndexChunker chunker(chunkDir, itemsInChunk);

    FileInArchive inArchive(dataFileName);

    while (!inArchive.eof()) {
        DataHeader dataHeader;
        dataHeader.deserialize(inArchive);

        assert(dataHeader.canary == DataHeader::CANARY);

        chunker.add(IndexEntry(dataHeader.key, inArchive.pos()));

        inArchive.skip(dataHeader.dataSize);

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
        sortFileInMemory<CopyableFileInArchive, CopyableFileOutArchive, IndexEntry>(*fileNameIt);

        std::cout << "Chunk " << *fileNameIt << " sorted\n";
    }

    std::cout << "Done\n";
}

void mergeChunks(const std::list<std::string>& chunkFiles, const char* outputFileName) {
    std::cout << "Merging chunks...\n";

    std::list<CopyableFileInArchive> indexArchives;

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        indexArchives.push_back(CopyableFileInArchive(*fileNameIt));
    }

    IndexMerger merger(indexArchives);
    FileOutArchive outArchive(outputFileName);
    merger.merge([&outArchive](const IndexEntry& entry) -> void {
        entry.serialize(outArchive);
    });

    std::cout << "Index created\n";
}

void createIndex(const char* dataFileName, const char* chunkDir, const char* outputFileName) {
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
        createIndex(dataFileName, chunkDir, outputFileName);
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
