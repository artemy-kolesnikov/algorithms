#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <sys/time.h>
#include <stdint.h>

size_t getRadix(int val, int num) {
	return (val >> (num << 2 << 1)) & 0xFF;
}

size_t getBitPos(uint32_t n) {
   size_t pos = 0;
   if(n & 0xFFFF0000) pos = 16, n >>= 16;
   if(n & 0x0000FF00) pos += 8, n >>= 8;
   if(n & 0x000000F0) pos += 4, n >>= 4;
   if(n & 0x0000000C) pos += 2, n >>= 2;
   if(n & 0x00000002) pos += 1;
   return pos;
}

size_t getRandValue() {
    int result = 0;

    result |= (rand() % 255) << 0;
    result |= (rand() % 255) << 8;
    result |= (rand() % 255) << 16;
    result |= (rand() % 255) << 24;

    return result;
}

void radixSort(std::vector<uint32_t>& array) {
    const size_t RADIX = 4;
    const size_t COUNTS_SIZE = 0x101;
    const size_t ARRAY_SIZE = array.size();

    const size_t SMALLSET_POW2 = 1 << getBitPos(ARRAY_SIZE);
    const size_t REST_SIZE = ARRAY_SIZE - SMALLSET_POW2;

    uint32_t tmpArray[ARRAY_SIZE];
    memset(tmpArray, 0, ARRAY_SIZE * sizeof(uint32_t));

    uint32_t* sorted = &array.front();
    uint32_t* buffer = tmpArray;

    uint32_t counts1[COUNTS_SIZE];
    uint32_t counts2[COUNTS_SIZE];
    memset(counts1, 0, COUNTS_SIZE * sizeof(uint32_t));
    memset(counts2, 0, COUNTS_SIZE * sizeof(uint32_t));

    uint32_t* currentCounts = counts1;
    uint32_t* prevCounts = counts2;

    for (size_t i = 0; i < SMALLSET_POW2;) {
        ++(currentCounts[getRadix(sorted[i++], 0) + 1]);
        ++(currentCounts[getRadix(sorted[i++], 0) + 1]);
        ++(currentCounts[getRadix(sorted[i++], 0) + 1]);
        ++(currentCounts[getRadix(sorted[i++], 0) + 1]);
    }

    for (size_t i = SMALLSET_POW2; i < ARRAY_SIZE; ++i) {
        ++(currentCounts[getRadix(sorted[i], 0) + 1]);
    }

    for (size_t i = 1; i < COUNTS_SIZE;) {
        currentCounts[i++] += currentCounts[i - 1];
        currentCounts[i++] += currentCounts[i - 1];
        currentCounts[i++] += currentCounts[i - 1];
        currentCounts[i++] += currentCounts[i - 1];
    }

    for (uint8_t r = 0; r < RADIX - 1; ++r) {
        for (size_t i = 0; i < SMALLSET_POW2;) {
            size_t tmpArrayIndex = (currentCounts[getRadix(sorted[i], r)])++;
            buffer[tmpArrayIndex] = sorted[i];
            ++(prevCounts[getRadix(buffer[tmpArrayIndex], r + 1) + 1]);

            ++i;

            tmpArrayIndex = (currentCounts[getRadix(sorted[i], r)])++;
            buffer[tmpArrayIndex] = sorted[i];
            ++(prevCounts[getRadix(buffer[tmpArrayIndex], r + 1) + 1]);

            ++i;

            tmpArrayIndex = (currentCounts[getRadix(sorted[i], r)])++;
            buffer[tmpArrayIndex] = sorted[i];
            ++(prevCounts[getRadix(buffer[tmpArrayIndex], r + 1) + 1]);

            ++i;

            tmpArrayIndex = (currentCounts[getRadix(sorted[i], r)])++;
            buffer[tmpArrayIndex] = sorted[i];
            ++(prevCounts[getRadix(buffer[tmpArrayIndex], r + 1) + 1]);

            ++i;
        }

        for (size_t i = SMALLSET_POW2; i < ARRAY_SIZE; ++i) {
            size_t tmpArrayIndex = (currentCounts[getRadix(sorted[i], r)])++;
            buffer[tmpArrayIndex] = sorted[i];

            ++(prevCounts[getRadix(buffer[tmpArrayIndex], r + 1) + 1]);
        }

        for (size_t i = 1; i < COUNTS_SIZE;) {
            prevCounts[i++] += prevCounts[i - 1];
            prevCounts[i++] += prevCounts[i - 1];
            prevCounts[i++] += prevCounts[i - 1];
            prevCounts[i++] += prevCounts[i - 1];
        }

        memset(currentCounts, 0, COUNTS_SIZE * sizeof(uint32_t));

        std::swap(currentCounts, prevCounts);
        std::swap(sorted, buffer);
    }

    for (size_t i = 0; i < SMALLSET_POW2;) {
        buffer[(currentCounts[getRadix(sorted[i++], RADIX - 1)])++] = sorted[i - 1];
        buffer[(currentCounts[getRadix(sorted[i++], RADIX - 1)])++] = sorted[i - 1];
        buffer[(currentCounts[getRadix(sorted[i++], RADIX - 1)])++] = sorted[i - 1];
        buffer[(currentCounts[getRadix(sorted[i++], RADIX - 1)])++] = sorted[i - 1];
    }

    for (size_t i = SMALLSET_POW2; i < ARRAY_SIZE; ++i) {
        buffer[(currentCounts[getRadix(sorted[i], RADIX - 1)])++] = sorted[i];
    }
}

std::pair<float, float> evaluate(size_t size) {
    float radixTime = .0;
    float qsortTime = .0;

    float predRadixTime = .0;
    float predQsortTime = .0;

    struct timeval start, end;

    const size_t SIZE = size;
    for (size_t i = 0; i < 10000; ++i) {
        std::vector<uint32_t> vec1(SIZE);
        std::vector<uint32_t> vec2(SIZE);
        for (size_t i = 0; i < SIZE; ++i) {
            int value = getRandValue();
            vec1[i] = value;
            vec2[i] = value;
        }

        gettimeofday(&start, NULL);
        radixSort(vec1);
        gettimeofday(&end, NULL);

        int seconds  = end.tv_sec  - start.tv_sec;
        int useconds = end.tv_usec - start.tv_usec;

        predRadixTime = radixTime;
        radixTime += ((seconds) * 1000 + useconds/1000.0);

        assert(predRadixTime <= radixTime);

        gettimeofday(&start, NULL);
        std::sort(vec2.begin(), vec2.end());
        gettimeofday(&end, NULL);

        seconds  = end.tv_sec  - start.tv_sec;
        useconds = end.tv_usec - start.tv_usec;

        predQsortTime = qsortTime;
        qsortTime += ((seconds) * 1000 + useconds/1000.0);

        assert(predQsortTime <= qsortTime);

        assert(vec1 == vec2);
    }

    return std::make_pair(radixTime, qsortTime);
}

int main(int argc, char* argv[]) {
    for (size_t i = 30; i <= 60; i += 1) {
        std::pair<float, float> result = evaluate(i);
        std::cout << i << " " << result.first << " " << result.second << "\n";
    }

    return 0;
}
