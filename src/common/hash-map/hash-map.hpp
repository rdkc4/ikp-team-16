#ifndef HASH_MAP_HPP
#define HASH_MAP_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <functional>

/// initial number of buckets.
constexpr size_t DEFAULT_MAP_CAPACITY = 16;

/**
 * @class hash_map
 * @brief implementation of the hash_map; using constant number of buckets and chaining.
 * K - type of the key.
 * V - type of the value.
 * Hash - hash function; defaults to std::hash<K>.
*/
template<typename K, typename V, typename Hash = std::hash<K>>
class hash_map {
private:
    /** 
     * @struct hash_map_entry
     * @brief the structure of the element inside of the hash_map.
    */
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

    /// list containing the linked-list of entries.
    hash_map_entry** buckets;
    /// number of entries in the hash_map.
    size_t size;
    /// number of buckets.
    size_t capacity;
    /// hash function for key hashing.
    Hash hash_function;

    /** 
     * @brief calculates the index of the bucket.
     * @param key - const reference to a key.
     * @returns index of the bucket.
    */
    size_t calculate_bucket(const K& key) const noexcept {
        return hash_function(key) % capacity;
    }

    /**
     * @brief frees all entries from a bucket.
     * @param head - pointer to the first element of the bucket linked list.
    */
    void delete_entries_from_bucket(hash_map_entry* head){
        while(head){
            hash_map_entry* temp = head;
            head = head-> next;
            delete temp;
        }
    }

    /**
     * @brief frees all entries from each bucket.
    */
    void clear_buckets(){
        for(size_t i = 0; i < capacity; ++i){
            delete_entries_from_bucket(buckets[i]);
        }
    }

public:
    /**
     * @brief creates the instance of the hash_map.
     * @details allocates DEFAULT_MAP_CAPACITY buckets.
     * size defaults to 0.
     * capacity set to DEFAULT_MAP_CAPACITY.
     * sets each bucket linked list to nullptr.
    */
    hash_map() :
        buckets(static_cast<hash_map_entry**>(::operator new (sizeof(hash_map_entry*) * DEFAULT_MAP_CAPACITY))),
        size(0),
        capacity(DEFAULT_MAP_CAPACITY) {
            
        for(size_t i = 0; i < capacity; ++i){
            buckets[i] = nullptr;
        }
    }

    /**
     * @brief creates the instance of the hash_map.
     * @param hash_map_capacity - number of buckets.
     * @throws std::invalid_argument if hash_map_capacity is 0.
     * @details allocates hash_map_capacity buckets.
     * size defaults to 0.
     * sets each bucket linked list to nullptr.
    */
    hash_map(size_t hash_map_capacity) :
        buckets(static_cast<hash_map_entry**>(::operator new (sizeof(hash_map_entry*) * hash_map_capacity))),
        size(0),
        capacity(hash_map_capacity) {
        
        if(hash_map_capacity == 0){
            throw std::invalid_argument("Invalid hash map capacity");
        }

        for(size_t i = 0; i < capacity; ++i){
            buckets[i] = nullptr;
        }
    }    

    /**
     * @brief frees the memory allocated for the buckets.
    */
    ~hash_map() {
        clear_buckets();
        ::operator delete(buckets);
    }

    /// deleted copy constructor.
    hash_map(const hash_map&) = delete;

    /// deleted assignment operator.
    hash_map& operator=(const hash_map&) = delete;

    /**
     * @brief constructs the hash_map instance from an existing one.
     * @param other - rvalue of the existing hash_map.
     * @details moves the ownership of the buckets, size and capacity from other to this.
    */
    hash_map(hash_map&& other) noexcept :
        buckets(std::exchange(other.buckets, nullptr)),
        size(std::exchange(other.size, 0)),
        capacity(std::exchange(other.capacity, 0)) {}

    /**
     * @brief constructs new hash_map by assigning it an existing one.
     * @param other - rvalue of the existing hash_map.
     * @details moves the ownership of the buckets, size and capacity from other to this.
    */
    hash_map& operator=(hash_map&& other) noexcept {
        if(this != &other){
            clear_buckets();
            ::operator delete(buckets);

            buckets = std::exchange(other.buckets, nullptr);
            size = std::exchange(other.size, 0);
            capacity = std::exchange(other.capacity, 0);
        }
        return *this;
    }

    /**
     * @brief inserts new (key, value) pair; or updates value if key already exists.
     * @param key - const reference to a key.
     * @param value - const reference to a value.
    */
    void insert(const K& key, const V& value){
        size_t bucket_idx = calculate_bucket(key);
        hash_map_entry* current = buckets[bucket_idx];

        while(current){
            if(current->key == key){
                current->value = value;
                return;
            }
            current = current->next;
        }

        hash_map_entry* new_entry = new hash_map_entry(key, value);
        new_entry->next = buckets[bucket_idx];
        buckets[bucket_idx] = new_entry;
        ++size;
    }

    /**
     * @brief inserts new (key, value) pair; or updates value if key already exists.
     * @param key - rvalue of the key.
     * @param value - rvalue of the value.
    */
    void insert(K&& key, V&& value) {
        size_t bucket_idx = calculate_bucket(key);
        hash_map_entry* current = buckets[bucket_idx];

        while(current){
            if(current->key == key){
                current->value = std::move(value);
                return;
            }
            current = current->next;
        }

        hash_map_entry* new_entry = new hash_map_entry(std::move(key), std::move(value));
        new_entry->next = buckets[bucket_idx];
        buckets[bucket_idx] = new_entry;
        ++size;
    }

    /**
     * @brief looks up element in a hash_map.
     * @param key - const reference to a key.
     * @returns pointer to the value associated with the key; nullptr if not found.
    */
    V* find(const K& key) noexcept {
        size_t bucket_idx = calculate_bucket(key);
        hash_map_entry* current = buckets[bucket_idx];

        while(current) {
            if(current->key == key){
                return &current->value;
            }
            current = current->next;
        }
        return nullptr;
    }

    /**
     * @brief looks up element in a hash_map.
     * @param key - const reference to a key.
     * @returns const pointer to the value associated with the key; nullptr if not found.
    */
    const V* find(const K& key) const noexcept {
        size_t bucket_idx = calculate_bucket(key);
        hash_map_entry* current = buckets[bucket_idx];

        while(current){
            if(current->key == key){
                return & current->value;
            }
            current = current->next;
        }
        return nullptr;
    }

    /**
     * @brief removes the (key, value) pair from the hash_map.
     * @param key - const reference to a key.
     * @returns true if element was erased; false otherwise.
    */
    bool erase(const K& key){
        size_t bucket_idx = calculate_bucket(key);
        hash_map_entry* current = buckets[bucket_idx];
        hash_map_entry* previous = nullptr;

        while(current){
            if(current->key == key){
                if(previous){
                    previous->next = current->next;
                }
                else {
                    buckets[bucket_idx] = current->next;
                }
                delete current;
                --size;
                return true;
            }
            previous = current;
            current = current->next;
        }
        return false;
    }

    /**
     * @brief looks up if element exists in a hash_map.
     * @param key - const reference to a key.
     * @returns true if hash_map contains element; false otherwise.
    */
    bool contains(const K& key) const noexcept {
        return find(key) != nullptr;
    }

    /**
     * @brief gets the value for the specific key.
     * @param key - const reference to a key of the element.
     * @returns reference to a value.
     * @throws std::out_of_range if key doesn't exist.
    */
    V& operator[](const K& key){
        V* value = find(key);
        if(!value){
            throw std::out_of_range("Key not found");
        }
        return *value;
    }

    /**
     * @brief gets the value for the specific key.
     * @param key - const reference to a key of the element.
     * @returns reference to a value.
     * @throws std::out_of_range if key doesn't exist.
    */
    const V& operator[](const K& key) const {
        const V* value = find(key);
        if(!value){
            throw std::out_of_range("Key not found");
        }
        return *value;
    }

    /**
     * @brief gets the size of the hash_map.
     * @returns number of entries in the hash_map.
    */
    size_t get_size() const noexcept {
        return size;
    }

    /**
     * @brief gets the capacity of the hash_map.
     * @returns number of buckets in the hash_map.
    */
    size_t get_capacity() const noexcept {
        return capacity;
    }

    /** 
     * @brief checks if the hash_map is empty.
     * @returns true if hash_map is empty; false otherwise.
    */
    bool empty() const noexcept {
        return size == 0;
    }

    /**
     * @brief clears all entries from the hash_map.
    */
    void clear() {
        clear_buckets();
        for(size_t i = 0; i < capacity; ++i){
            buckets[i] = nullptr;
        }
        size = 0;
    }

};

#endif