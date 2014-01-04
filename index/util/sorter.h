#pragma once

#include <algorithm>
#include <vector>
#include <functional>

template <typename InArchive, typename OutArchive, typename EntryType>
void sortFileInMemory(const std::string& fileName) {
    InArchive inArchive(fileName);

    std::vector<EntryType> dataVector;

    while (!inArchive.eof()) {
        EntryType entry;
        entry.deserialize(inArchive);

        dataVector.push_back(entry);
    }

    std::sort(dataVector.begin(), dataVector.end());

    OutArchive outArchive(fileName);
    std::for_each(dataVector.begin(), dataVector.end(), [&outArchive](const EntryType& entry) {
        entry.serialize(outArchive);
    });
}
