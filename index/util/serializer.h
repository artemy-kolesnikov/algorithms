#pragma once

#include <type_traits>

template <typename T>
struct IsClassSerializable {
    static const bool value = false;
};

template <typename Item, typename OutArchive>
void serialize(const Item& item, OutArchive& out, typename std::enable_if<!IsClassSerializable<Item>::value>::type * = 0) {
    out.write(item);
}

template <typename Item, typename InArchive>
void deserialize(Item& item, InArchive& in, typename std::enable_if<!IsClassSerializable<Item>::value>::type * = 0) {
    in.read(item);
}

template <typename Item>
bool isValid(const Item&, typename std::enable_if<!IsClassSerializable<Item>::value>::type * = 0) {
    return true;
}

template <typename Item, typename OutArchive>
void serialize(const Item& item, OutArchive& out, typename std::enable_if<IsClassSerializable<Item>::value>::type * = 0) {
    item.serialize(out);
}

template <typename Item, typename InArchive>
void deserialize(Item& item, InArchive& out, typename std::enable_if<IsClassSerializable<Item>::value>::type * = 0) {
    item.deserialize(out);
}

template <typename Item>
bool isValid(const Item& item, typename std::enable_if<IsClassSerializable<Item>::value>::type * = 0) {
    return item.isValid();
}
