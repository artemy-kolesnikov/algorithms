#pragma once

#include <fstream>
#include <list>
#include <queue>

template <typename ItemType, typename InArchive>
class Merger {
    struct ItemHolder {
        ItemType item;
        InArchive *inArchive;

        ItemHolder(InArchive *archive) : inArchive(archive) {}

        bool operator < (const ItemHolder& other) const {
            return !(item < other.item);
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
    void merge(ProcessFunction process) {
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

private:
    std::list<InArchive> inArchives;
};

template <typename ItemType, typename InArchive>
class TwoWayMerger {
public:
    TwoWayMerger(const InArchive& firstArc, const InArchive& secondArc) :
            firstArchive(firstArc),
            secondArchive(secondArc) {}

    template <typename ProcessFunction>
    void merge(ProcessFunction process) {
        ItemType first, second;

        deserialize(first, firstArchive);
        deserialize(second, secondArchive);

        bool hasInFirst = false;
        bool hasInSecond = false;

        while (true) {
            if (first < second) {
                process(first);
                if (firstArchive.eof()) {
                    hasInFirst = false;
                    break;
                }
                deserialize(first, firstArchive);
                hasInFirst = true;
            } else {
                process(second);
                if (secondArchive.eof()) {
                    hasInSecond = false;
                    break;
                }
                deserialize(second, secondArchive);
                hasInSecond = true;
            }
        }

        if (hasInFirst) {
            process(first);
            passTail(firstArchive, process);
        } else if (hasInSecond) {
            process(second);
            passTail(secondArchive, process);
        }
    }

private:
    template <typename ProcessFunction>
    void passTail(InArchive& archive, ProcessFunction process) {
        while (!archive.eof()) {
            ItemType item;
            deserialize(item, archive);
            process(item);
        }
    }

private:
    InArchive firstArchive;
    InArchive secondArchive;
};
