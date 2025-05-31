#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    struct CommandBufferSubmitInfo {
        std::vector<VkSemaphore> wait_semaphores{};
        std::vector<VkPipelineStageFlags> wait_stages{};
        std::vector<VkSemaphore> signal_semaphores{};
        VkFence signal_fence{ VK_NULL_HANDLE };
        bool should_wait_idle{ false };
    };

    /**
     * Wrapper over VkCommandBuffer. Do not reuse the same command buffer.
     */
    class CommandBuffer {
        public:
        /**
         * Constructor
         * @param device Logical device that owns this command buffer
         * @param command_buffer Handle to existing command buffer
         */
        CommandBuffer(const LogicalDevice& device,
                      VkCommandBuffer command_buffer);

        CommandBuffer(const CommandBuffer& other) = delete;

        CommandBuffer(CommandBuffer&& other) = delete;

        CommandBuffer& operator=(const CommandBuffer& other) = delete;

        CommandBuffer& operator=(CommandBuffer&& other) = delete;

        VkCommandBuffer get() const;

        /**
       * Begin recording commands
       * @param usage Command buffer usage flags
       */
        void begin(VkCommandBufferUsageFlags usage,
                   VkCommandBufferInheritanceInfo inheritance_info = {});

        /**
       * Stop recording and submit the buffer
       */
        void end();

        void submit(const CommandBufferSubmitInfo& submit_info);

        VkCommandBuffer* get_handle_address();

        private:
        const LogicalDevice* device_;
        VkCommandBuffer command_buffer_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
