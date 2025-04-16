#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

#include "window.h"

#ifdef NDEBUG
    constexpr bool enable_validation_layers = false;
#else
constexpr bool enable_validation_layers = true;
#endif

const std::vector<const char*> validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

namespace njin::vulkan {
    /**
     * VkInstance wrapper
     */
    class Instance {
    public:
        Instance(const std::string& app_name,
                 std::vector<const char*>& extensions);

        Instance(const Instance& instance) = delete;

        Instance& operator=(const Instance&) = delete;

        Instance(Instance&& instance) = delete;

        Instance& operator=(Instance&& instance) = delete;

        ~Instance();

        /**
         * @return Handle to VkInstance
         */
        const VkInstance* const get() const;

    private:
        VkInstance instance_ = VK_NULL_HANDLE;
    };
}

