#pragma once

template <typename ForwardIterator>
class IteratorReader {
public:
    typedef typename ForwardIterator::value_type ValueType;

    IteratorReader(ForwardIterator first_, ForwardIterator last_) :
        first(first_),
        last(last_) {}

    bool read(ValueType& result) {
        if (first == last) {
            return false;
        }

        result = *first++;

        return true;
    }

    const ValueType* read() {
        if (!read(lastRead)) {
            return 0;
        }

        return &lastRead;
    }

private:
    ForwardIterator first;
    ForwardIterator last;
    ValueType lastRead;
};
