Index and external sort
=====

#Functionality

Data structure must be serializable and comparable, i.e. has functions listed below:

```cpp
struct Data {
    template <typename OutArchive>
    void serialize(OutArchive& out) const {
        out.write(..);
    }

    template <typename InArchive>
    void deserialize(InArchive& in) {
        in.read(..);
    }

    bool operator < (const Data& other) const {
        return ...;
    }
};
```

Library internals call global serialize/deserialize functions to have common interface for POD and non POD types
For non POD types client must make a template specialization like this:

```cpp
template <>
struct IsClassSerializable<Data> {
    static const bool value = true;
};
```

##Create index of data file

Index creation tool create several chunks of index entries then sort each chunk separatly in memory and then merge.

###Library usage example:

```cpp
struct IndexEntry {
    unsigned char key[64];
    uint64_t filePos;
    uint64_t canary;
};

const char* dataFileName = "data.dat";
const char* chunkDir = "/var/tmp/index";
const char* outputFileName = "index.dat";
const char* itemsInChunk = (1 << 20);
const size_t threadCount = 4;

createIndex<DataEntry, IndexEntry>(dataFileName, chunkDir, outputFileName,
        itemsInChunk, threadCount, [](const DataEntry& data, size_t filePos) {
    return IndexEntry(data.header.key, filePos);
});
```

###Utility usage example:

```
cmake .
make
create_test_data/create_test_data 10000000 data.dat
create_index/create_index create_test_data/data.dat tmp 1000000 4 index.dat
```

##External file sort

External sort uses tool create several chunks of data entries then sort each chunk separatly in memory and then merge.

###Library usage example:

```cpp
const char* dataFileName = "data.dat";
const char* chunkDir = "/var/tmp/index";
const char* outputFileName = "sorted.dat";
const char* itemsInChunk = (1 << 20);
const size_t threadCount = 4;

externalSort<DataEntry>(dataFileName, chunkDir, outputFileName, itemsInChunk, threadCount);
```

###Utility usage example:

```sh
cmake .
make
create_test_data/create_test_data 10000000 data.dat
sort/sort create_test_data/data.dat tmp 1000000 4 sorted.dat
```

#Folders

1. create_index        - Index creation tool
2. sort                - external sort
3. util                - Utility classes
4. create_test_data    - Test data creation tool
5. test_index          - Index check tool
