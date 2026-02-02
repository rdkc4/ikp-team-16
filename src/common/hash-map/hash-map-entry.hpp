#ifndef HASH_MAP_ENTRY_HPP
#define HASH_MAP_ENTRY_HPP

#include <utility>

/** 
 * @struct hash_map_entry
 * @brief the structure of the element inside of the hash_map.
*/
template<typename K, typename V>
struct hash_map_entry {
    /// key of the entry.
    K key;
    /// value of the entry.
    V value;
    /// pointer to the next entry in the bucket.
    hash_map_entry* next;

    /**
     * @brief creates an instance of the hash_map entry.
     * @param k - const reference to a key.
     * @param v - const reference to a value.
     * @details next defaults to nullptr.
    */
    hash_map_entry(const K& k, const V& v): key(k), value(v), next(nullptr) {}

    /**
     * @brief creates an instance of the hash_map entry.
     * @param k - rvalue key.
     * @param v - rvalue value.
     * @details next defaults to nullptr.
    */
    hash_map_entry(K&& k, V&& v) : key(std::move(k)), value(std::move(v)), next(nullptr) {}
};

#endif