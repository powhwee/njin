#include "vulkan/CommandPool.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace njin::vulkan {
    CommandPool::CommandPool(const LogicalDevice& device,
                             uint32_t queue_family_index) : device_{
        device.get()
    } {
        VkCommandPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue_family_index,
        };

        if (vkCreateCommandPool(device_, &info, nullptr, &command_pool_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    CommandPool::~CommandPool() {
        vkDestroyCommandPool(device_, command_pool_, nullptr);
    }

    VkCommandPool CommandPool::get() const {
        return command_pool_;
    }
}
