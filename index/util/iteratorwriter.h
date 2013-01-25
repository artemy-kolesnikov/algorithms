#pragma once

template <typename ForwardIterator>
class IteratorWriter {
public:
    typedef typename ForwardIterator::value_type ValueType;

    IteratorWriter(ForwardIterator iterator_) :
            iterator(iterator_) {}

    void write(const ValueType& value) {
        *iterator++ = value;
    }

    void write(const ValueType* value) {
        *iterator++ = *value;
    }

private:
    ForwardIterator iterator;
};
