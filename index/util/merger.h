#pragma once

#include <fstream>
#include <list>
#include <queue>

template <typename ItemType, typename ReaderType, typename WriterType>
class Merger {
    struct ItemHolder {
        const ItemType* item;
        ReaderType *reader;

        ItemHolder(ReaderType *r) : reader(r) {}

        bool operator < (const ItemHolder& other) const {
            return !(*item < *other.item);
        }

        bool readNextItem() {
            item = reader->read();
            return (item != 0);
        }
    };

    typedef std::priority_queue<ItemHolder> PriorityQueueType;

public:
    Merger(const std::list<ReaderType>& readers_, WriterType writer_) :
            readers(readers_),
            writer(writer_) {}

    void merge() {
        PriorityQueueType priorityQueue;

        typename std::list<ReaderType>::iterator readerIt = readers.begin();
        for (; readerIt != readers.end(); ++readerIt) {
            ItemHolder holder(&*readerIt);
            if (holder.readNextItem()) {
                priorityQueue.push(holder);
            }
        }

        while (!priorityQueue.empty()) {
            ItemHolder holder = priorityQueue.top();
            priorityQueue.pop();

            writer.write(*holder.item);

            if (holder.readNextItem()) {
                if (priorityQueue.empty()) {
                    do {
                        writer.write(*holder.item);
                    } while (holder.readNextItem());
                } else {
                    priorityQueue.push(holder);
                }
            }
        }
    }

private:
    std::list<ReaderType> readers;
    WriterType writer;
};
