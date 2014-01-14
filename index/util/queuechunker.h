#pragma once

#include <filearchive.h>
#include <noncopyable.h>

#include <memory>

template <typename EntryType>
class QueueChunker : Noncopyable {
    struct ItemHolder {
        EntryType* item;
        bool isSignalItem;

        ItemHolder(EntryType* ptr = nullptr, bool isSignal = false) :
                item(ptr),
                isSignalItem(isSignal) {}

        bool operator < (const ItemHolder& other) const {
            if (isSignalItem) {
                return false;
            }

            return !(*item < *other.item);
        }
    };

public:
    QueueChunker(const std::string& chnkDir, size_t qSize) :
            chunkDir(chnkDir),
            queueSize(qSize),
            chunkCounter(0),
            hasSignalItem(false) {
        std::string chunkFileName = getChunkFileName();
        chunkFileNames.push_back(chunkFileName);
        fileArchive.reset(new FileOutArchive(chunkFileName));

        itemMemory.reserve(queueSize);
    }

    void add(const EntryType& entry) {
        if (queue.size() < queueSize) {
            itemMemory.push_back(entry);
            queue.push(ItemHolder(&itemMemory.back()));
            prevTop = *queue.top().item;
        } else {
            ItemHolder holder = queue.top();

            if (holder.isSignalItem) {
                createNextFileArchive();
                hasSignalItem = false;
            } else {
                assert(prevTop <= *holder.item);
            }

            serialize(*holder.item, *fileArchive.get());
            *holder.item = entry;
            queue.pop();

            if (!hasSignalItem && prevTop < entry) {
                queue.push(ItemHolder(holder.item, true));
                hasSignalItem = true;
            } else {
                queue.push(ItemHolder(holder.item));
            }

            prevTop = *queue.top().item;
        }
    }

    const std::list<std::string> getChunkFileNames() const {
        return chunkFileNames;
    }

private:
    std::string getChunkFileName() const {
        std::stringstream sstr;
        sstr << chunkDir << "/chunk_" << chunkCounter << ".dat";
        return sstr.str();
    }

    void createNextFileArchive() {
        ++chunkCounter;

        std::string chunkFileName = getChunkFileName();
        chunkFileNames.push_back(chunkFileName);
        fileArchive.reset(new FileOutArchive(chunkFileName));
    }

private:
    const std::string chunkDir;
    size_t queueSize;
    size_t chunkCounter;
    bool hasSignalItem;
    std::list<std::string> chunkFileNames;
    std::shared_ptr<FileOutArchive> fileArchive;
    std::priority_queue<ItemHolder> queue;
    std::vector<EntryType> itemMemory;
    EntryType prevTop;
};
