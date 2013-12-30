#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <vector>

#include <iterator>

size_t getRandValue() {
    int result = 0;

    result |= (rand() % 255) << 0;
    result |= (rand() % 255) << 8;
    result |= (rand() % 255) << 16;
    result |= (rand() % 255) << 24;

    return result;
}

template <typename RandomAcessIterator>
void radix_sort(RandomAcessIterator begin, RandomAcessIterator end) {
    typedef typename RandomAcessIterator::value_type ValueType;

    const size_t RADIX = sizeof(ValueType);
    const size_t SIZE = end - begin;

    const size_t MAX_STACK_ARRAY_SIZE = 0xFF;
    ValueType stackTmpArray[MAX_STACK_ARRAY_SIZE];

    std::vector<ValueType> tmpVector;

    ValueType* srcRef = &*begin;
    ValueType* auxRef = 0;

    if (SIZE <= MAX_STACK_ARRAY_SIZE) {
        auxRef = stackTmpArray;
    } else {
        tmpVector.resize(SIZE);
        auxRef = &tmpVector[0];
    }

    const size_t COUNT_SIZE = 0x100;

    uint32_t counts[COUNT_SIZE][RADIX] = {0};

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(srcRef);
    for (size_t i = 0; i < SIZE; ++i) {
        for (uint8_t r = 0; r < RADIX; ++r, ++ptr) {
            ++counts[*ptr + 1][r];
        }
    }

    uint32_t* countsPtr = reinterpret_cast<uint32_t*>(counts);
    for (size_t i = 0; i < (COUNT_SIZE - 1)* RADIX; ++i, ++countsPtr) {
        *(countsPtr + RADIX) += *countsPtr;
    }

    for (uint8_t r = 0; r < RADIX; ++r) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(srcRef) + r;
        for (size_t i = 0; i < SIZE; ++i, ptr += RADIX) {
            auxRef[counts[*ptr][r]++] = srcRef[i];
        }

        std::swap(srcRef, auxRef);
    }

    if (RADIX == 1) {
        std::copy(srcRef, srcRef + SIZE, begin);
    }
}

std::pair<float, float> evaluate(size_t size) {
    float radixTime = .0;
    float qsortTime = .0;

    float predRadixTime = .0;
    float predQsortTime = .0;

    struct timeval start, end;

    typedef uint32_t ValueType;

    const size_t SIZE = size;
    for (size_t i = 0; i < 10000; ++i) {
        std::vector<ValueType> vec1(SIZE);
        std::vector<ValueType> vec2(SIZE);
        std::vector<ValueType> vec(SIZE);
        for (size_t i = 0; i < SIZE; ++i) {
            ValueType value = getRandValue();
            vec[i] = value;
        }

        std::copy(vec.begin(), vec.end(), vec1.begin());
        std::copy(vec.begin(), vec.end(), vec2.begin());

        gettimeofday(&start, NULL);
        radix_sort(vec1.begin(), vec1.end());
        gettimeofday(&end, NULL);

        int seconds  = end.tv_sec  - start.tv_sec;
        int useconds = end.tv_usec - start.tv_usec;

        predRadixTime = radixTime;
        radixTime += ((seconds) * pow(10, 6) + useconds);

        assert(predRadixTime <= radixTime);

        gettimeofday(&start, NULL);
        std::sort(vec2.begin(), vec2.end());
        gettimeofday(&end, NULL);

        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;

        predQsortTime = qsortTime;
        qsortTime += (seconds * pow(10, 6) + useconds);

        assert(predQsortTime <= qsortTime);

        assert(vec1 == vec2);
    }

    return std::make_pair(radixTime, qsortTime);
}

int main(int argc, char* argv[]) {
    for (size_t i = 0; i <= 100; ++i) {
        std::pair<float, float> result = evaluate(i);
        std::cout << i << " " << result.first << " " << result.second << "\n";
        if (result.first < result.second) {
            //break;
        }
    }

    return 0;
}
