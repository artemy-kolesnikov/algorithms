#pragma once

#include <algorithm>
#include <vector>
#include <type_traits>

#include <xmmintrin.h>
#include <emmintrin.h>

template <typename RandomAcessIterator>
void radix_sort(RandomAcessIterator begin, RandomAcessIterator end,
        typename std::enable_if<std::is_integral<typename RandomAcessIterator::value_type>::value>::type* = 0) {
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

    const size_t COUNT_SIZE = 0x101;

    uint32_t counts[COUNT_SIZE][RADIX] = {0};

    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(srcRef);
    for (size_t i = 0; i < SIZE; ++i) {
        for (uint8_t r = 0; r < RADIX; ++r, ++ptr) {
            ++counts[*ptr + 1][r];
        }
    }

    for (size_t i = 0; i < COUNT_SIZE - 1; ++i) {
        __m128i v0 = _mm_load_si128((__m128i*)&counts[i][0]);
        __m128i v1 = _mm_load_si128((__m128i*)&counts[i + 1][0]);

        __m128i vsum = _mm_add_epi32(v0, v1);
        _mm_store_si128((__m128i*)&counts[i + 1][0], vsum);
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
