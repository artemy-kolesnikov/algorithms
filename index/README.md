Index
=====

Create index of data file using external memory algorithms
Data file has to have this format:

struct DataHeader {
    unsigned char key[64];
    uint64_t flags;
    uint64_t data;
    uint64_t size;
};

After header there are size bytes of data then next header and so on.
Index creation tool create several chunks of index entries then sort each chunk separatly in memory and then merge.
Index entry has this format:

struct IndexEntry {
    unsigned char key[64];
    uint64_t filePos;
    uint64_t canary;
};

create_index        - Index creation tool
create_test_data    - Test data creation tool
performance_test    - Algorithms performance tests
test_index          - Index check tool
util                - Utility classes
