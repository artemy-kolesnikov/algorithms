#pragma once

#include <algorithm>
#include <vector>
#include <functional>

namespace _Impl {

template <typename WriterType>
struct WriteOperation : public std::unary_function<typename WriterType::EntryType, void> {
    explicit WriteOperation(WriterType* writer_) :
            writer(writer_) {}

    void operator()(const typename WriterType::EntryType& item) {
        writer->write(item);
    }

    WriterType* writer;
};

}

template <typename ReaderType, typename WriterType>
void sortFileInMemory(const std::string& fileName) {
    typedef typename ReaderType::EntryType EntryType;

    ReaderType reader(fileName);

    std::vector<EntryType> dataVector;

    EntryType entry;
    while (reader.read(entry)) {
        dataVector.push_back(entry);
    }

    std::sort(dataVector.begin(), dataVector.end());

    WriterType writer(fileName);
    _Impl::WriteOperation<WriterType> writeOp(&writer);
    std::for_each(dataVector.begin(), dataVector.end(), writeOp);
}
