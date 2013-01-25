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

/*template <typename ReaderType, typename WriterType>
class InMemorySorter {
public:
    //typedef std::vector<ItemType> Collection;

    explicit InMemorySorter(const std::string& fName) :
            fileName(fName) {}

    void sortToFile(const std::string& fileName) {
        std::sort(collection.begin(), collection.end());

        WriterType writer(fileName);
        _Impl::WriteOperation<WriterType, ItemType> writeOp(&writer);
        std::for_each(collection.begin(), collection.end(), writeOp);
    }*/

    /*const Collection& data() const {
        return collection;
    }

    template <typename Item>
    void add(const Item& item) {
        collection.push_back(item);
    }

    size_t itemCount() const {
        return collection.size();
    }

    void clear() {
        collection.clear();
    }*/

/*private:
    std::string fileName;
};*/
