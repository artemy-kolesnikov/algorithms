#include <iostream>

#include <exception.h>
#include <filearchive.h>
#include <index.h>
#include <serializer.h>
#include <benchmarkdata.h>

namespace {

void printUsage() {
    std::cout << "Usage: test_result [index|sorted] file_name\n";
}

template <typename Entry>
void test(const char* fileName) {
    Entry prev;

    FileInArchive inArchive(fileName);

    size_t count = 0;

    while (!inArchive.eof()) {
        Entry entry;
        deserialize(entry, inArchive);

        if (!isValid(entry)) {
            throw Exception() << "Failed data in" << count << "position";
        }

        if (count && entry < prev) {
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
            test<BenchmarkData>(fileName);
        } else if (entryType == "index") {
            test<IndexEntry>(fileName);
        } else {
            throw Exception() << "Unknown entry type" << entryType;
        }
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
