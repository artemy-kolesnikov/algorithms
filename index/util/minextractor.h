#pragma once

#include <exception.h>

#include <cmath>
#include <vector>

template <typename T>
class MinExtractor {
public:
    template <typename ForwardIterator>
    MinExtractor(ForwardIterator begin, ForwardIterator end) :
            array(begin, end),
            maskedCount(0) {
        if (array.empty()) {
            throw Exception() << "Emtpy data";
        }

        tree.resize(array.size() + getNodeCount());
        maskBits.resize(tree.size());

        precalcData(tree.size());

        for (size_t i = 0; i < array.size(); ++i) {
            tree[i] = i;
        }

        for (size_t i = 0; i < tree.size() - 1; i += 2) {
            size_t leftLeafIndex = i;
            size_t rightLeafIndex = i + 1;

            assert(parent(leftLeafIndex) == parent(rightLeafIndex));

            size_t parentIndex = parent(leftLeafIndex);

            if (rightLeafIndex < tree.size()) {
                tree[parentIndex] = tree[minDataIndex(tree[leftLeafIndex], tree[rightLeafIndex])];
            }
        }
    }

    const T& min() const {
        return array[tree.back()];
    }

    void changeMin(const T& value) {
        size_t minIndex = tree.back();
        array[minIndex] = value;
        propogate(minIndex);
    }

    void maskMin() {
        size_t minIndex = tree.back();
        maskBits[minIndex] = true;
        propogate(minIndex);
        ++maskedCount;
    }

    bool empty() const {
        return maskedCount == array.size();
    }

private:
    size_t invertIndex(size_t index) const {
        return tree.size() - 1 - index;
    }

    size_t parent(size_t index) const {
        return parents[index];
    }

    size_t sibling(size_t index) const {
        if (isRightLeaf(index)) {
            return index - 1;
        } else {
            return index + 1;
        }
    }

    bool isRightLeaf(size_t index) const {
        return index & 1;
    }

    void propogate(size_t index) {
        while (index < tree.size() - 1) {
            size_t siblingIndex = sibling(index);

            assert(parent(index) == parent(siblingIndex));

            size_t parentIndex = parent(index);

            if (maskBits[index] && maskBits[siblingIndex]) {
                maskBits[parentIndex] = true;
            } else if (maskBits[index]) {
                tree[parentIndex] = tree[siblingIndex];
            } else if (maskBits[siblingIndex]) {
                tree[parentIndex] = tree[index];
            } else {
                tree[parentIndex] = tree[minDataIndex(tree[index], tree[siblingIndex])];
            }


            index = parentIndex;
        }
    }

    size_t getNodeCount() const {
        size_t treeHeight = ceil(log2f(array.size()));
        size_t levelNodeCount = floor(array.size() / 2.0);

        size_t countOdd = (array.size() % 2) == 0 ? 0 : 1;

        size_t nodeCount = 0;
        for (size_t i = 0; i < treeHeight; ++i) {
            nodeCount += levelNodeCount;
            if ((levelNodeCount  + countOdd) % 2 == 0) {
                levelNodeCount = (levelNodeCount + countOdd)/2.0;
                countOdd = 0;
            } else {
                levelNodeCount = levelNodeCount/2.0;
                countOdd = 1;
            }
        }

        return nodeCount;
    }

    size_t minDataIndex(size_t firstIndex, size_t secondIndex) const {
        if (array[firstIndex] < array[secondIndex]) {
            return firstIndex;
        }

        return secondIndex;
    }

    void precalcData(size_t treeSize) {
        parents.resize(treeSize);
        for (size_t i = 0; i < treeSize; ++i) {
            parents[i] = invertIndex(ceil(invertIndex(i)/2.0) - 1);
        }
    }

private:
    std::vector<T> array;
    std::vector<size_t> tree;
    std::vector<bool> maskBits;
    size_t maskedCount;
    std::vector<size_t> parents;
};
