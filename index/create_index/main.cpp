#include <iostream>
#include <stdexcept>

#include <index.h>

namespace {

void printUsage() {
    std::cout << "Usage: create_index data_file_name chunk_dir items_in_chunk thread_count out_file_name\n";
}

}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        printUsage();
        return 1;
    }

    const char* dataFileName = argv[1];
    const char* chunkDir = argv[2];
    size_t itemsInChunk = atoi(argv[3]);
    size_t threadCount = atoi(argv[4]);
    const char* outputFileName = argv[5];

    try {
        createIndex<DataEntry, IndexEntry>(dataFileName, chunkDir, outputFileName,
                itemsInChunk, threadCount, [](const DataEntry& data, size_t filePos) {
            return IndexEntry(data.header.key, filePos);
        });
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
