#pragma once

#include <algorithm>
#include <vector>
#include <type_traits>
#include <memory>

#include <xmmintrin.h>
#include <emmintrin.h>

template <typename RandomAcessIterator, typename GetRadix>
void radix_sort(RandomAcessIterator begin, RandomAcessIterator end,
        GetRadix getRadix, size_t radixCount = 4) {
    typedef typename RandomAcessIterator::value_type ValueType;

    const size_t RADIX = radixCount;
    const size_t SIZE = end - begin;

    ValueType* srcRef = &*begin;
    /*ValueType* auxRef = 0;

    std::vector<ValueType> tmpVector(SIZE);
    auxRef = &tmpVector[0];*/

    std::unique_ptr<char[]> tmpMemory(new char[SIZE * sizeof(ValueType)]);
    ValueType* auxRef = reinterpret_cast<ValueType*>(tmpMemory.get());

    const size_t COUNT_SIZE = 0x10000;

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

    /*for (size_t i = 0; i < COUNT_SIZE - 1; ++i) {
        __m128i v0 = _mm_load_si128((__m128i*)&counts[i][0]);
        __m128i v1 = _mm_load_si128((__m128i*)&counts[i + 1][0]);

        __m128i vsum = _mm_add_epi32(v0, v1);
        _mm_store_si128((__m128i*)&counts[i + 1][0], vsum);
    }*/

    for (int8_t r = 0; r < RADIX; ++r) {
        for (size_t i = 0; i < SIZE; ++i) {
            size_t index = counts[getRadix(srcRef[i], r)][r]++;
            if (r == 0) {
                //tmpVector.emplace(tmpVector.begin() + index, srcRef[i]);
                new(&auxRef[index]) ValueType(std::move(srcRef[i]));
            } else {
                auxRef[index] = std::move(srcRef[i]);
            }
        }

        std::swap(srcRef, auxRef);
    }

    if (RADIX % 2 == 1) {
        std::move(srcRef, srcRef + SIZE, begin);
    }
}
