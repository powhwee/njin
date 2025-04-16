#pragma once

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

        ~CommandPool();

        VkCommandPool get() const;

    private:
        const VkDevice device_{ VK_NULL_HANDLE };
        VkCommandPool command_pool_{ VK_NULL_HANDLE };
    };
}
