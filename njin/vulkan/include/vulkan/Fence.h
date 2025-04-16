#pragma once
#include <vulkan/vulkan_core.h>
#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    /**
     * Wrapper around VkFence
     */
    class Fence {
    public:
        Fence(const LogicalDevice& device);

        Fence(Fence const& other) = delete;

        Fence& operator=(Fence const& other) = delete;

        Fence(Fence&& other) = delete;

        Fence& operator=(Fence&& other) = delete;

        VkFence* get_handle_address();

        VkFence get() const;

        ~Fence();

    private:
        const VkDevice device_{ VK_NULL_HANDLE };
        VkFence fence_{ VK_NULL_HANDLE };
    };
}
