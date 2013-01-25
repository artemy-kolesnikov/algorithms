Index
=====

#Functionality

Create index of data file using external memory algorithms.
Data file has to have this format:

```cpp
struct DataHeader {
    unsigned char key[64];
    uint64_t flags;
    uint64_t data;
    uint64_t size;
};
```

After header there are size bytes of data then next header and so on.
Index creation tool create several chunks of index entries then sort each chunk separatly in memory and then merge.
Index entry has this format:

```cpp
struct IndexEntry {
    unsigned char key[64];
    uint64_t filePos;
    uint64_t canary;
};
```

#Folders

1. create_index        - Index creation tool
2. create_test_data    - Test data creation tool
3. performance_test    - Algorithms performance tests
4. test_index          - Index check tool
5. util                - Utility classes
