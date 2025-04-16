#pragma once
#include <string>
#include <vector>

#include <map>

namespace njin::core {
    template<typename T>
    class njRegistry {
        public:
        void add(const std::string& name, const T& item);
        T* get(const std::string& name);
        const T* get(const std::string& name) const;
        /**
         * Retrieve a list of all records in the registry
         * @return List of items
         * @note invalidated when an item is removed or added after this is called
         */
        std::vector<const T*> get_records() const;

        /**
         * Get a map of user provided texture names to the texture resources
         * @return Map of texture names to texture resources
         */
        std::map<std::string, const T*> get_map() const;

        int get_record_count() const;
        private:
        std::map<std::string, T> registry_{};
    };
}  // namespace njin::core

#include "core/njRegistry.tpp"