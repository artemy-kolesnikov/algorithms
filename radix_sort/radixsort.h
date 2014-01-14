#pragma once

#include <algorithm>
#include <vector>
#include <type_traits>

template <typename RandomAcessIterator, typename GetRadix>
void radix_sort(RandomAcessIterator begin, RandomAcessIterator end,
        GetRadix getRadix, size_t radixCount = 4) {
    typedef typename RandomAcessIterator::value_type ValueType;

    const size_t RADIX = radixCount;
    const size_t SIZE = end - begin;

    ValueType* srcRef = &*begin;
    ValueType* auxRef = 0;

    std::vector<ValueType> tmpVector(SIZE);
    auxRef = &tmpVector[0];

    const size_t COUNT_SIZE = 0x101;

    uint32_t counts[COUNT_SIZE][RADIX];
    memset(counts, 0, COUNT_SIZE * RADIX * sizeof(uint32_t));

    for (size_t i = 0; i < SIZE; ++i) {
        for (uint8_t r = 0; r < RADIX; ++r) {
            ++counts[getRadix(srcRef[i], r) + 1][r];
        }
    }

    uint32_t* countsPtr = reinterpret_cast<uint32_t*>(counts);
    for (size_t i = 0; i < (COUNT_SIZE - 1)* RADIX; ++i, ++countsPtr) {
        *(countsPtr + RADIX) += *countsPtr;
    }

    for (uint8_t r = 0; r < RADIX; ++r) {
        for (size_t i = 0; i < SIZE; ++i) {
            auxRef[counts[getRadix(srcRef[i], r)][r]++] = std::move(srcRef[i]);
        }

        std::swap(srcRef, auxRef);
    }
}
