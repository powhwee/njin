#pragma once

#include "vulkan/CommandPool.h"

namespace njin::vulkan {
    /**
     * Wrapper over VkCommandBuffer. Do not reuse the same command buffer.
     */
    class CommandBuffer {
        public:
        /**
         * Constructor
         * @param device Logical device to allocate this command buffer on
         * @param command_pool Command pool to allocate a command buffer from
         */
        CommandBuffer(const LogicalDevice& device,
                      const CommandPool& command_pool);

        CommandBuffer(const CommandBuffer& other) = delete;

        CommandBuffer(CommandBuffer&& other) = delete;

        CommandBuffer& operator=(const CommandBuffer& other) = delete;

        CommandBuffer& operator=(CommandBuffer&& other) = delete;

        VkCommandBuffer get() const;

        /**
       * Begin recording commands
       */
        void begin();

        /**
       * Stop recording and submit the buffer
       */
        void end();

        VkCommandBuffer* get_handle_address();

        private:
        const LogicalDevice* device_;
        VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
