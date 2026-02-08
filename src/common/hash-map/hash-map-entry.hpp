#ifndef HASH_MAP_ENTRY_HPP
#define HASH_MAP_ENTRY_HPP

#include <utility>

/** 
 * @struct hash_map_entry
 * @brief the structure of the element inside of the hash_map.
 * @tparam K - type of the key.
 * @tparam V - type of the value.
*/
template<typename K, typename V>
struct hash_map_entry {
    /// pointer to the next entry in the bucket.
    hash_map_entry* next;

    /// key of the entry.
    K key;

    /// value of the entry.
    V value;

    /**
     * @brief creates an instance of the hash_map entry.
     * @param k - const reference to a key.
     * @param v - const reference to a value.
     * @details next defaults to nullptr.
    */
    hash_map_entry(const K& k, const V& v): next(nullptr), key(k), value(v) {}

    /**
     * @brief creates an instance of the hash_map entry.
     * @param k - rvalue key.
     * @param v - rvalue value.
     * @details next defaults to nullptr.
    */
    hash_map_entry(K&& k, V&& v) : next(nullptr), key(std::move(k)), value(std::move(v)) {}
};

#endif