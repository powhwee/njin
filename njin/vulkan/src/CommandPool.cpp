#include "vulkan/CommandPool.h"

#include <stdexcept>

#include <vulkan/vulkan_core.h>

namespace njin::vulkan {
    CommandPool::CommandPool(const LogicalDevice& device,
                             uint32_t queue_family_index) :
        device_{ &device } {
        VkCommandPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queue_family_index,
        };

        if (vkCreateCommandPool(device_->get(),
                                &info,
                                nullptr,
                                &command_pool_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    CommandBuffer CommandPool::allocate_buffer(VkCommandBufferLevel level) {
        VkCommandBufferAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool_,
            .level = level,
            .commandBufferCount = 1,
        };

        VkCommandBuffer command_buffer;
        if (vkAllocateCommandBuffers(device_->get(), &info, &command_buffer) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffer!");
        }

        allocated_buffers_.push_back(command_buffer);

        return { *device_, command_buffer };
    }

    void CommandPool::free_buffers() {
        if (!allocated_buffers_.empty()) {
            vkFreeCommandBuffers(device_->get(),
                                 command_pool_,
                                 static_cast<uint32_t>(allocated_buffers_
                                                       .size()),
                                 allocated_buffers_.data());
        }
        allocated_buffers_.clear();
    }

    CommandPool::~CommandPool() {
        vkDestroyCommandPool(device_->get(), command_pool_, nullptr);
    }

    VkCommandPool CommandPool::get() const {
        return command_pool_;
    }
}  // namespace njin::vulkan
