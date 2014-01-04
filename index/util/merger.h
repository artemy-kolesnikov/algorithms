#pragma once

#include <fstream>
#include <list>
#include <queue>

template <typename ItemType, typename InArchive, typename OutArchive>
class Merger {
    struct ItemHolder {
        ItemType item;
        InArchive *inArchive;

        ItemHolder(InArchive *archive) : inArchive(archive) {}

        bool operator < (const ItemHolder& other) const {
            return !(item < other.item);
        }

        bool readNextItem() {
            item.deserialize(*inArchive);
            return !inArchive->eof();
        }
    };

    typedef std::priority_queue<ItemHolder> PriorityQueueType;

public:
    Merger(const std::list<InArchive>& archives, const OutArchive& outArchive_) :
            inArchives(archives),
            outArchive(outArchive_) {}

    void merge() {
        PriorityQueueType priorityQueue;

        typename std::list<InArchive>::iterator archIt = inArchives.begin();
        for (; archIt != inArchives.end(); ++archIt) {
            ItemHolder holder(&*archIt);
            if (holder.readNextItem()) {
                priorityQueue.push(holder);
            }
        }

        while (!priorityQueue.empty()) {
            ItemHolder holder = priorityQueue.top();
            priorityQueue.pop();

            holder.item.serialize(outArchive);

            if (holder.readNextItem()) {
                if (priorityQueue.empty()) {
                    do {
                        holder.item.serialize(outArchive);
                    } while (holder.readNextItem());
                } else {
                    priorityQueue.push(holder);
                }
            }
        }
    }

private:
    std::list<InArchive> inArchives;
    OutArchive outArchive;
};
