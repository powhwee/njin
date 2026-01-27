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
        const T& get(uint32_t index) const;

        /**
         * Get the primary resource name for a given alias.
         * Searches for the first resource that starts with "{alias}-".
         * @param alias The alias to search for (e.g., "cube")
         * @return The full resource name (e.g., "cube-Object_0")
         * @throws std::runtime_error if no matching resource is found
         */
        std::string get_primary_mesh_name(const std::string& alias) const;
        
        /**
         * Get all resource names for a given alias.
         * Searches for all resources that start with "{alias}-".
         * @param alias The alias to search for (e.g., "cube")
         * @return List of full resource names
         */
        std::vector<std::string> get_all_mesh_names(const std::string& alias) const;

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
        std::vector<T*> indexed_registry_{};
    };
}  // namespace njin::core

#include "core/njRegistry.tpp"