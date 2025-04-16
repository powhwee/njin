#include "vulkan/Fence.h"

#include <stdexcept>

namespace njin::vulkan {
    Fence::Fence(const LogicalDevice& device) : device_{ device.get() } {
        VkFenceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        if (vkCreateFence(device_, &info, nullptr, &fence_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create fence");
        }
    }

    VkFence* Fence::get_handle_address() {
        return &fence_;
    }

    VkFence Fence::get() const {
        return fence_;
    }

    Fence::~Fence() {
        vkDestroyFence(device_, fence_, nullptr);
    }
}
