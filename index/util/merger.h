#pragma once

#include <fstream>
#include <list>
#include <queue>
#include <vector>

template <typename ItemType, typename InArchive>
class Merger {
    struct ItemHolder {
        ItemType* item;
        InArchive *inArchive;

        ItemHolder(ItemType* itemPtr, InArchive *archive) :
                item(itemPtr),
                inArchive(archive) {}

        bool operator < (const ItemHolder& other) const {
            // Make min heap
            return !(*item < *other.item);
        }

        bool readNextItem() {
            //Save archive eof state because read operation can change it
            bool eof = inArchive->eof();
            deserialize(*item, *inArchive);
            return !eof;
        }
    };

    typedef std::priority_queue<ItemHolder> PriorityQueue;

public:
    explicit Merger(const std::list<InArchive>& archives) :
            inArchives(archives) {}

    template <typename ProcessFunction>
    void merge(ProcessFunction process) {
        PriorityQueue priorityQueue;

        std::vector<ItemType> itemMemory;
        itemMemory.reserve(inArchives.size());

        typename std::list<InArchive>::iterator archIt = inArchives.begin();
        for (; archIt != inArchives.end(); ++archIt) {
            itemMemory.push_back(ItemType());

            ItemHolder holder(&itemMemory.back(), &*archIt);
            if (holder.readNextItem()) {
                priorityQueue.push(holder);
            }
        }

        while (!priorityQueue.empty()) {
            ItemHolder holder = priorityQueue.top();
            priorityQueue.pop();

            process(*holder.item);

            const bool queueEmpty = priorityQueue.empty();
            const ItemHolder& topHolder = priorityQueue.top();
            bool holderRead = holder.readNextItem();
            while (!queueEmpty && holderRead && *holder.item < *topHolder.item) {
                process(*holder.item);
                holderRead = holder.readNextItem();
            }

            if (holderRead) {
                if (priorityQueue.empty()) {
                    do {
                        process(*holder.item);
                    } while (holder.readNextItem());
                } else {
                    priorityQueue.push(holder);
                }
            }
        }
    }

private:
    std::list<InArchive> inArchives;
};
