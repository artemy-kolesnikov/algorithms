#pragma once

#include <fstream>
#include <list>
#include <queue>
#include <minextractor.h>

template <typename ItemType, typename InArchive>
class Merger {
    struct ItemHolder {
        ItemType item;
        InArchive *inArchive;

        ItemHolder(InArchive *archive) : inArchive(archive) {}

        bool operator < (const ItemHolder& other) const {
            return item < other.item;
        }

        bool readNextItem() {
            bool eof = inArchive->eof();
            deserialize(item, *inArchive);
            return !eof;
        }
    };

    typedef std::priority_queue<ItemHolder> PriorityQueue;

public:
    explicit Merger(const std::list<InArchive>& archives) :
            inArchives(archives) {}

    template <typename ProcessFunction>
    void merge2(ProcessFunction process) {
        PriorityQueue priorityQueue;

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

            process(holder.item);

            if (holder.readNextItem()) {
                if (priorityQueue.empty()) {
                    do {
                        process(holder.item);
                    } while (holder.readNextItem());
                } else {
                    priorityQueue.push(holder);
                }
            }
        }
    }

    template <typename ProcessFunction>
    void merge(ProcessFunction process) {
        std::list<ItemHolder> items;

        typename std::list<InArchive>::iterator archIt = inArchives.begin();
        for (; archIt != inArchives.end(); ++archIt) {
            ItemHolder holder(&*archIt);
            if (holder.readNextItem()) {
                items.push_back(holder);
            }
        }

        MinExtractor<ItemHolder> minExtractor(items.begin(), items.end());

        while (!minExtractor.empty()) {
            ItemHolder holder = minExtractor.min();
            process(holder.item);
            if (holder.readNextItem()) {
                minExtractor.changeMin(holder);
            } else {
                minExtractor.maskMin();
            }
        }
    }

private:
    std::list<InArchive> inArchives;
};
