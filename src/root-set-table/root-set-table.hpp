#ifndef ROOT_SET_TABLE_HPP
#define ROOT_SET_TABLE_HPP

#include <string>
#include <cstddef>
#include <utility>

#include "../common/hash-map/hash-map.hpp"
#include "../common/root-set/root-set-base.hpp"

/**
 * @class root_set_table
 * @brief manages root sets for garbage collection.
*/
class root_set_table {
private:
    /// hash map storing root sets by name.
    hash_map<std::string, root_set_base> roots;
    
public:
    /*
     * @brief creates an instance of the root set table.
    */
    root_set_table() = default;

    /**
     * @brief deletes the instance of the root set table.
    */
    ~root_set_table() = default;

    /// deleted copy constructor.
    root_set_table(const root_set_table&) = delete;

    /// deleted assignment operator.
    root_set_table& operator=(const root_set_table&) = delete;

    /**
     * @brief constructs new root set table from an existing one.
     * @param other - rvalue of the existing root set value.
     * @details moves ownership of the roots from other to this.
    */
    root_set_table(root_set_table&& other) noexcept = default;

    /**
     * @brief constructs new root set table by assigning it an existing one.
     * @param other - rvalue of the existing root set table.
     * @details moves ownership of the roots from other to this.
    */
    root_set_table& operator=(root_set_table&& other) noexcept = default;

    /**
     * @brief adds new root to the root set table.
     * @tparam KK Key forwarding type.
     * @tparam VV Value forwarding type.
     * @param key - name of the root.
     * @param root - instance of the root set entry.
     * @returns void
    */
    template<typename KK, typename VV>
    void add_root(KK&& key, VV&& root) {
        roots.insert(std::forward<KK>(key), std::forward<VV>(root));
    }

    /**
     * @brief removes root from the root set table.
     * @param key - name of the root.
     * @returns void
    */
    void remove_root(const std::string& key) {
        roots.erase(key);
    }

    /**
     * @brief getter for the root from the root set table.
     * @param key - name of the root.
     * @returns pointer to a root set entry.
    */
    root_set_base* get_root(const std::string& key) noexcept {
        return roots.find(key);
    }

    /**
     * @brief getter for the root from the root set table.
     * @param key - name of the root.
     * @returns const pointer to a root set entry.
    */
    const root_set_base* get_root(const std::string& key) const noexcept {
        return roots.find(key);
    }

    /**
     * @brief removes all roots from root set table.
     * @returns void
    */
    void clear() noexcept {
        roots.clear();
    }

    /**
     * @brief getter for the number of roots in the root set table.
     * @returns number of roots.
    */
    size_t get_root_count() const noexcept {
        return roots.get_size();
    }

};

#endif