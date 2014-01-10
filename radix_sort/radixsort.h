#pragma once

#include <algorithm>
#include <vector>

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
