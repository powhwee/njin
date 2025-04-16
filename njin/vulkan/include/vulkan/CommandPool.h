#pragma once

#include "vulkan/CommandBuffer.h"
#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    class CommandPool {
        public:
        /**
         * Constructor
         * @param device Logical device to create the command pool on
         * @param queue_family_index Index of queue family this command pool's
         * buffers to submit to
         */
        CommandPool(const LogicalDevice& device, uint32_t queue_family_index);

        CommandPool(const CommandPool& other) = delete;

        CommandPool(const CommandPool&& other) = delete;

        CommandPool& operator=(const CommandPool& other) = delete;

        CommandPool& operator=(const CommandPool&& other) = delete;

        CommandBuffer allocate_buffer(VkCommandBufferLevel level);

        ~CommandPool();

        VkCommandPool get() const;

        private:
        const LogicalDevice* device_;
        VkCommandPool command_pool_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
