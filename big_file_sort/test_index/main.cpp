#include <iostream>

#include <index.h>
#include <filereader.h>
#include <exception>

namespace {

void printUsage() {
    std::cout << "Usage: test_index index_file_name\n";
}

void testIndex(const char* indexFileName) {
    Key prevKey;

    FileReader<IndexEntry> indexFileReader(indexFileName, 1024);

    size_t count = 0;

    IndexEntry indexEntry;
    while (indexFileReader.read(indexEntry)) {
        if (indexEntry.canary != DataHeader::CANARY) {
            throw Exception() << "Failed index data in" << count << "position";
        }

        if (prevKey > indexEntry.key) {
            throw Exception() << "Failed index order in" << count << "position";
        }

        prevKey = indexEntry.key;

        ++count;
    }

    std::cout << "Index is correct, " << count << " items in index\n";
}

}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printUsage();
        return 1;
    }

    try {
        testIndex(argv[1]);
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
