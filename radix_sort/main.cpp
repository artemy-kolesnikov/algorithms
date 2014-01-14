#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdint.h>
#include <sys/time.h>
#include <vector>
#include <sstream>
#include <iterator>

#include <radixsort.h>

const size_t RAND_STRING_SIZE = 10;

template <typename T>
T getRandValue();

template <>
uint32_t getRandValue<uint32_t>() {
    size_t result = 0;

    result |= (rand() % 256) << 0;
    result |= (rand() % 256) << 8;
    result |= (rand() % 256) << 16;
    result |= (rand() % 256) << 24;

    return result;
}

template <>
std::string getRandValue<std::string>() {
    std::string result;
    for (size_t i = 0; i < RAND_STRING_SIZE; ++i) {
        std::stringstream sstr;
        sstr << rand() % 255;
        result += sstr.str();
    }

    return result;
}

template <>
std::vector<uint8_t> getRandValue<std::vector<uint8_t>>() {
    std::vector<uint8_t> result(RAND_STRING_SIZE);
    for (size_t i = 0; i < RAND_STRING_SIZE; ++i) {
        result[i] = rand() % 255;
    }

    return result;
}

std::pair<float, float> evaluate(size_t size) {
    float radixTime = .0;
    float qsortTime = .0;

    float predRadixTime = .0;
    float predQsortTime = .0;

    struct timeval start, end;

    //typedef std::string ValueType;
    typedef std::vector<uint8_t> ValueType;
    //typedef uint32_t ValueType;

    const size_t SIZE = size;
    for (size_t i = 0; i < 10000; ++i) {
        std::vector<ValueType> vec1(SIZE);
        std::vector<ValueType> vec2(SIZE);
        std::vector<ValueType> vec(SIZE);
        for (size_t i = 0; i < SIZE; ++i) {
            ValueType value = getRandValue<ValueType>();
            vec[i] = value;
        }

        std::copy(vec.begin(), vec.end(), vec1.begin());
        std::copy(vec.begin(), vec.end(), vec2.begin());

        gettimeofday(&start, NULL);
        radix_sort(vec1.begin(), vec1.end(), [](const ValueType& value, uint32_t radix) -> uint8_t {
            return value[RAND_STRING_SIZE - 1 - radix];
        }, RAND_STRING_SIZE);
        /*radix_sort(vec1.begin(), vec1.end(), [](const ValueType& value, uint32_t radix) -> uint32_t {
            return (value >> (8 * radix)) & 0xFF;
        });*/
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

        if (vec1 != vec2) {
            std::cout << "Incorrect sorting result\n";
            exit(1);
        }
    }

    return std::make_pair(radixTime, qsortTime);
}

int main(int argc, char* argv[]) {
    for (size_t i = 200; i <= 1000; ++i) {
        std::pair<float, float> result = evaluate(i);
        std::cout << i << " " << result.first << " " << result.second << "\n";
        if (result.first < result.second) {
            break;
        }
    }

    return 0;
}
