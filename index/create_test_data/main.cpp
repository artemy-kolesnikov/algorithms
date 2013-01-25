#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <archive.h>
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
    std::ofstream out(outFileName, std::ios::out | std::ios::binary);

    if (!out.is_open()) {
        throw Exception() << "Can't open file" << outFileName;
    }

    std::vector<unsigned char> data;

    for (size_t i = 0; i < recordCount; ++i) {
        DataHeader dataHeader(0, rand() % 100);
        getRandKey(dataHeader.key);
        data.resize(dataHeader.size, 0);

        OutArchive outArchive(DataHeader::bytesUsed());
        dataHeader.serialize(outArchive);
        out.write(&outArchive.getBuffer().front(), outArchive.getBuffer().size());

        out.write(reinterpret_cast<const char*>(&data[0]), data.size());
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
