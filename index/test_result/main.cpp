#include <iostream>

#include <index.h>
#include <filearchive.h>
#include <exception>

namespace {

void printUsage() {
    std::cout << "Usage: test_result [index|sorted] file_name\n";
}

template <typename Entry>
void testIndex(const char* indexFileName) {
    Entry prev;

    FileInArchive indexFileArchive(indexFileName);

    size_t count = 0;

    while (true) {
        Entry entry;
        entry.deserialize(indexFileArchive);

        if (indexFileArchive.eof()) {
            break;
        }

        if (!entry.isValid()) {
            throw Exception() << "Failed data in" << count << "position";
        }

        if (entry < prev) {
            throw Exception() << "Failed order in" << count << "position";
        }

        prev = entry;

        ++count;
    }

    std::cout << "Data is correct, " << count << " items\n";
}

}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printUsage();
        return 1;
    }

    std::string entryType = argv[1];
    const char* fileName = argv[2];

    try {
        if (entryType == "sorted") {
            testIndex<DataEntry>(fileName);
        } else if (entryType == "index") {
            testIndex<IndexEntry>(fileName);
        } else {
            throw Exception() << "Unknown entry type" << entryType;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
