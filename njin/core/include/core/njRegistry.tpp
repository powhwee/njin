#include <format>
#include <iostream>
#include <string>

namespace njin::core {
    template<typename T>
    void njRegistry<T>::add(const std::string& name, const T& item) {
        if (registry_.contains(name)) {
            std::cout << std::format("Item with name '{}' already exists.",
                                     name)
                      << std::endl;
            return;
        }
        registry_.insert({ name, item });
    }

    template<typename T>
    T* njRegistry<T>::get(const std::string& name) {
        if (!registry_.contains(name)) {
            std::cout << std::format("Item with name '{}' does not exist.",
                                     name)
                      << std::endl;
            return nullptr;
        }
        return &registry_.at(name);
    }

    template<typename T>
    const T* njRegistry<T>::get(const std::string& name) const {
        if (!registry_.contains(name)) {
            std::cout << std::format("Item with name '{}' does not exist.",
                                     name)
                      << std::endl;
            return nullptr;
        }
        return &registry_.at(name);
    }

    template<typename T>
    std::vector<const T*> njRegistry<T>::get_records() const {
        std::vector<const T*> result{};
        for (auto it{ registry_.begin() }; it != registry_.end(); ++it) {
            result.push_back(&(it->second));
        }
        return result;
    }

    template<typename T>
    std::map<std::string, const T*> njRegistry<T>::get_map() const {
        std::map<std::string, const T*> result{};
        for (auto it{ registry_.begin() }; it != registry_.end(); ++it) {
            result[it->first] = &(it->second);
        }
        return result;
    }

    template<typename T>
    int njRegistry<T>::get_record_count() const {
        return registry_.size();
    }

}  // namespace njin::core