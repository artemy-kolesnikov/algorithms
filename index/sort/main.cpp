#include <iostream>

#include <benchmarkdata.h>
#include <sorter.h>
#include <radixsort.h>

#include <sys/time.h>

namespace {

void printUsage() {
    std::cout << "Usage: sort_file data_file_name tmp_data_dir items_in_chunk thread_count out_file_name\n";
}

struct EventCallback {
    void operator()(SortEventType type, int param) {
        switch (type) {
            case BeginCreatingChunks:
                std::cout << "Creating chunks..." << "\n";
                break;
            case DoneCreatingChunks:
                std::cout << param << " chunks created\n";
                break;
            case BeginSortingChunks:
                std::cout << "Sorting chunks..." << "\n";
                break;
            case ChunkSorted:
                std::cout << "Chunk " << param << " sorted\n";
                break;
            case EndSortingChunks:
                std::cout << "Done" << "\n";
                break;
            case BeginMergingChunks:
                std::cout << "Merging chunks..." << "\n";
                break;
            case DoneMergingChunks:
                std::cout << "Done" << "\n";
                break;
        }
    }
};

struct Sort {
    template <typename RandomAccessIterator>
    void operator()(RandomAccessIterator begin, RandomAccessIterator end) {
        struct timeval start, stop;
        gettimeofday(&start, NULL);

        std::sort(begin, end);
        /*radix_sort(begin, end, [](const typename RandomAccessIterator::value_type& value, uint32_t radix) -> uint8_t {
            return value.key[10 - 1 - radix];
        }, 10);*/

        gettimeofday(&stop, NULL);

        int seconds  = stop.tv_sec  - start.tv_sec;
        int useconds = stop.tv_usec - start.tv_usec;
        std::cout << ((seconds) * pow(10, 6) + useconds) << "\n";
    }
};

void test(const std::string& fileName) {
    FileInArchive inArchive(fileName);

    std::vector<BenchmarkData> vector1, vector2;

    while (!inArchive.eof()) {
        BenchmarkData entry;
        deserialize(entry, inArchive);

        vector1.push_back(entry);
        vector2.push_back(entry);
    }

    struct timeval start, stop;
    gettimeofday(&start, NULL);

    radix_sort(vector1.begin(), vector1.end(), [](const BenchmarkData& value, uint32_t radix) -> uint8_t {
        return value.key[10 - 1 - radix];
    }, 10);

    gettimeofday(&stop, NULL);

    int seconds  = stop.tv_sec  - start.tv_sec;
    int useconds = stop.tv_usec - start.tv_usec;

    std::cout << ((seconds) * pow(10, 6) + useconds) << "\n";

    gettimeofday(&start, NULL);

    std::sort(vector2.begin(), vector2.end());

    gettimeofday(&stop, NULL);

    seconds  = stop.tv_sec  - start.tv_sec;
    useconds = stop.tv_usec - start.tv_usec;

    std::cout << ((seconds) * pow(10, 6) + useconds) << "\n";
}

}

int main(int argc, char* argv[]) {
    test(argv[1]);

    return 0;

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
        externalSort<BenchmarkData>(dataFileName, chunkDir, outputFileName,
                itemsInChunk, threadCount, Sort(), EventCallback());
    } catch (std::exception& ex) {
        std::cerr << ex.what() << "\n";
        return 1;
    }

    return 0;
}
