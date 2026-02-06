#include "root-set-table.hpp"

#include <utility>

void root_set_table::add_root(std::string key, std::unique_ptr<root_set_base> root) {
    roots.insert(std::move(key), std::move(root));
}

void root_set_table::remove_root(const std::string& key) {
    roots.erase(key);
}

root_set_base* root_set_table::get_root(const std::string& key) noexcept {
    auto* entry = roots.find(key);
    return entry ? entry->get() : nullptr;
}

const root_set_base* root_set_table::get_root(const std::string& key) const noexcept {
    auto* entry = roots.find(key);
    return entry ? entry->get() : nullptr;
}

hash_map<std::string, std::unique_ptr<root_set_base>>& root_set_table::get_roots() noexcept {
    return roots;
}

const hash_map<std::string, std::unique_ptr<root_set_base>>& root_set_table::get_roots() const noexcept {
    return roots;
}

void root_set_table::clear() noexcept {
    roots.clear();
}

size_t root_set_table::get_root_count() const noexcept {
    return roots.get_size();
}