#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <filearchive.h>
#include <data.h>
#include <exception.h>

namespace {

void printUsage() {
    std::cout << "Usage: create_test_data record_count out_file_name\n";
}

void getRandKey(Key& key) {
    for (size_t i = 0; i < key.size(); ++i) {
        key[i] = rand() % 255;
    }
}

void createTestData(const char* outFileName, size_t recordCount) {

    FileOutArchive archive(outFileName);
    for (size_t i = 0; i < recordCount; ++i) {
        std::vector<char> data(rand() % 100);

        DataHeader dataHeader(0, data.size());
        getRandKey(dataHeader.key);

        DataEntry entry(dataHeader, data);

        entry.serialize(archive);
    }
}

}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    size_t recordCount = atoi(argv[1]);

    try {
        createTestData(argv[2], recordCount);
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
