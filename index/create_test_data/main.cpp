#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <archive.h>
#include <data.h>
#include <exception.h>
#include <filewriter.h>

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
    FileWriter<DataEntry> writer(outFileName);

    for (size_t i = 0; i < recordCount; ++i) {
        DataHeader dataHeader(0, rand() % 100);
        getRandKey(dataHeader.key);

        DataEntry entry(dataHeader, std::vector<char>(dataHeader.size));
        writer.write(entry);
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
