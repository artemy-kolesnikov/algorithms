#include <iostream>
#include <stdexcept>

#include <index.h>

namespace {

void printUsage() {
    std::cout << "Usage: create_index data_file_name chunk_dir out_file_name\n";
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
        createIndex<DataEntry, IndexEntry>(dataFileName, chunkDir, outputFileName, 1 << 20, [](const DataEntry& data, size_t filePos) {
            return IndexEntry(data.header.key, filePos);
        });
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
