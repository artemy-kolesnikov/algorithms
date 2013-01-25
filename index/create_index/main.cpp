#include <cassert>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <cmath>
#include <boost/shared_ptr.hpp>
#include <sstream>
#include <sys/time.h>

#include <archive.h>
#include <chunker.h>
#include <data.h>
#include <filereader.h>
#include <filewriter.h>
#include <index.h>
#include <merger.h>
#include <mmapper.h>
#include <sorter.h>

namespace {

void printUsage() {
    std::cout << "Usage: create_index data_file_name out_file_name\n";
}

typedef FileReader<IndexEntry> IndexFileReader;
typedef FileWriter<IndexEntry> IndexFileWriter;

typedef CopyableFileReader<IndexFileReader> CopyableIndexFileReader;
typedef CopyableFileWriter<IndexFileWriter> CopyableIndexFileWriter;

typedef Merger<IndexEntry, CopyableIndexFileReader, CopyableIndexFileWriter> IndexMerger;

typedef Chunker<IndexEntry> IndexChunker;

void createChunks(const char* dataFileName, const char* chunkDir, std::list<std::string>& chunkFiles) {
    const size_t INDEX_CHUNK_COUNT = (1 << 20);

    std::vector<char> dataHeaderBuffer(DataHeader::bytesUsed(), 0);
    InArchive inArchive(&dataHeaderBuffer.front(), &dataHeaderBuffer.front() + dataHeaderBuffer.size());

    std::cout << "Creating chunks...\n";

    size_t count = 0;

    IndexChunker chunker(chunkDir, INDEX_CHUNK_COUNT);

    ReadOnlyMemMapper mmapper(dataFileName);

    mmapper.map();

    const char* beginPtr = mmapper.getBeginPtr();
    const char* endPtr = mmapper.getEndPtr();

    const char* ptr = beginPtr;

    while (ptr < endPtr) {
        DataHeader dataHeader;
        inArchive.setBuffer(ptr, ptr + DataHeader::bytesUsed());

        dataHeader.deserialize(inArchive);

        assert(dataHeader.canary == DataHeader::CANARY);

        chunker.add(IndexEntry(dataHeader.key, ptr - beginPtr));

        ptr += DataHeader::bytesUsed() + dataHeader.size;

        ++count;
    }

    std::cout << "Done\n";

    std::cout << count << " data items" << "\n";

    chunker.flush();

    chunkFiles = chunker.getChunkFileNames();
}

void sortChunks(const std::list<std::string>& chunkFiles) {
    std::cout << "Sorting chunks\n";

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        sortFileInMemory<IndexFileReader, IndexFileWriter>(*fileNameIt);

        std::cout << "Chunk " << *fileNameIt << " sorted\n";
    }

    std::cout << "Done\n";
}

void mergeChunks(const std::list<std::string>& chunkFiles, const char* outputFileName) {
    std::cout << "Merging chunks...\n";

    std::list<CopyableIndexFileReader> indexReaders;

    std::list<std::string>::const_iterator fileNameIt = chunkFiles.begin();
    for (; fileNameIt != chunkFiles.end(); ++fileNameIt) {
        indexReaders.push_back(CopyableIndexFileReader(*fileNameIt));
    }

    CopyableIndexFileWriter indexWriter(outputFileName);
    IndexMerger merger(indexReaders, indexWriter);
    merger.merge();

    std::cout << "Index created\n";
}

void createIndex(const char* dataFileName, const char* chunkDir, const char* outputFileName) {
    std::list<std::string> chunkFiles;

    createChunks(dataFileName, chunkDir, chunkFiles);
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
