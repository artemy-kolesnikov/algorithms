#pragma once

#include <algorithm>
#include <vector>
#include <functional>

namespace _Impl {

template <typename WriterType, typename ItemType>
struct WriteOperation : public std::unary_function<ItemType, void> {
    explicit WriteOperation(WriterType* writer_) :
            writer(writer_) {}

    void operator()(ItemType item) {
        writer->write(item);
    }

    WriterType* writer;
};

}

template <typename ReaderType, typename WriterType>
void sortFileInMemory(const std::string& fileName) {
    typedef typename ReaderType::EntryType EntryType;

    std::vector<EntryType> dataVector;
    ReaderType reader(fileName, 1024);

    EntryType entry;
    while (reader.read(entry)) {
        assert(entry.canary == DataHeader::CANARY);

        dataVector.push_back(entry);
    }

    std::sort(dataVector.begin(), dataVector.end());

    WriterType writer(fileName, 1024);
    _Impl::WriteOperation<WriterType, EntryType> writeOp(&writer);
    std::for_each(dataVector.begin(), dataVector.end(), writeOp);
}
