#pragma once

#include <algorithm>
#include <vector>
#include <functional>

namespace _Impl {

template <typename OutArchive, typename EntryType>
struct WriteOperation : public std::unary_function<EntryType, void> {
    explicit WriteOperation(OutArchive* outArchive_) :
            outArchive(outArchive_) {}

    void operator()(const EntryType& item) {
        item.serialize(*outArchive);
    }

    OutArchive* outArchive;
};

}

template <typename InArchive, typename OutArchive, typename EntryType>
void sortFileInMemory(const std::string& fileName) {
    InArchive inArchive(fileName);

    std::vector<EntryType> dataVector;

    while (true) {
        EntryType entry;
        entry.deserialize(inArchive);

        if (inArchive.eof()) {
            break;
        }

        dataVector.push_back(entry);
    }

    std::sort(dataVector.begin(), dataVector.end());

    OutArchive outArchive(fileName);
    _Impl::WriteOperation<OutArchive, EntryType> writeOp(&outArchive);
    std::for_each(dataVector.begin(), dataVector.end(), writeOp);
}
