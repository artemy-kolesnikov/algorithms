#include <cmath>
#include <sys/time.h>
#include <vector>
#include <iostream>
#include <cassert>

#include <iteratorreader.h>
#include <iteratorwriter.h>
#include <merger.h>

struct TestStruct {
    TestStruct() {
        data = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    }

    TestStruct(int k) :
            key(k) {
        data = "111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
    }

    bool operator < (const TestStruct& other) const {
        return key < other.key;
    }

    bool operator > (const TestStruct& other) const {
        return key > other.key;
    }

    bool operator <= (const TestStruct& other) const {
        return key <= other.key;
    }

    int operator()() const {
        return key;
    }

    int key;
    std::string data;
};

void testMerge() {
    //const size_t MAX_ARRAYS = 1000;
    const size_t MAX_VECTOR_SIZE = 1000000;
    const size_t MAX_VALUE = 10000;

    typedef TestStruct ItemType;

    typedef std::vector<ItemType>::iterator VectorIterator;
    typedef std::vector<ItemType>::const_iterator VectorConstIterator;

    while (true) {
        /*size_t size = rand() % MAX_ARRAYS;
        if (!size) {
            size = 2;
        }*/

        size_t size = 2;

        std::list< IteratorReader<VectorConstIterator> > readers;

        std::list< std::vector<ItemType> > dataList;

        size_t outputSize = 0;

        for (size_t i = 0; i < size; ++i) {
            //size_t vectorSize = rand() % MAX_VECTOR_SIZE;
            size_t vectorSize = MAX_VECTOR_SIZE;

            outputSize += vectorSize;

            std::vector<ItemType> vec;
            for (size_t j = 0; j < vectorSize; ++j) {
                vec.push_back(ItemType(rand() % MAX_VALUE));
            }

            std::sort(vec.begin(), vec.end());

            /*std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, " "));
            std::cout << "\n";*/

            dataList.push_back(vec);
            readers.push_back(IteratorReader<VectorConstIterator>(dataList.back().begin(), dataList.back().end()));
        }

        std::vector<ItemType> output(outputSize, 0);
        IteratorWriter<VectorIterator> writer(output.begin());

        struct timeval start, end;

        //std::cout << "Start\n";

        gettimeofday(&start, NULL);

#if 1
        Merger<ItemType, IteratorReader<VectorConstIterator>, IteratorWriter<VectorIterator> > merger(readers, writer);
        merger.merge();
#else
        std::merge(dataList.front().begin(), dataList.front().end(),
                   dataList.back().begin(), dataList.back().end(),
                   output.begin());
#endif

        gettimeofday(&end, NULL);

        int seconds  = end.tv_sec  - start.tv_sec;
        int useconds = end.tv_usec - start.tv_usec;

        int time = ((seconds) * pow(10, 6) + useconds);

        std::cout << time << "\n";

        //assert(!readers.empty());
        //assert(!output.empty());

        /*std::cout << "output\n";
        std::copy(output.begin(), output.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";*/

        if (!output.empty()) {
            for (size_t i = 0; i < output.size() - 1; ++i) {
                assert(output[i] <= output[i + 1]);
            }
        } else {
            //std::cerr << "Empty result\n";
        }

        std::cout << "OK\n";

        //std::cout << "-------------------\n";
    }
}

int main(int argc, char* argv[]) {
    testMerge();

    return 0;
}
