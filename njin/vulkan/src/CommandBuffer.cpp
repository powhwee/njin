#include "vulkan/CommandBuffer.h"

#include <stdexcept>

#include <vulkan/vulkan_core.h>

#include "vulkan/GraphicsPipeline.h"

namespace njin::vulkan {
    CommandBuffer::CommandBuffer(const LogicalDevice& device,
                                 const CommandPool& command_pool) :
        device_{ &device } {
        VkCommandBufferAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool.get(),
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        if (vkAllocateCommandBuffers(device.get(), &info, &command_buffer_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffer!");
        };
    }

    VkCommandBuffer CommandBuffer::get() const {
        return command_buffer_;
    }

    void CommandBuffer::begin() {
        VkCommandBufferBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
            .pInheritanceInfo = nullptr
        };

        vkBeginCommandBuffer(command_buffer_, &info);
    }

    void CommandBuffer::end() {
        vkEndCommandBuffer(command_buffer_);

        VkSubmitInfo info{ .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                           .pNext = nullptr,
                           .waitSemaphoreCount = 0,
                           .pWaitSemaphores = nullptr,
                           .pWaitDstStageMask = 0,
                           .commandBufferCount = 1,
                           .pCommandBuffers = &command_buffer_,
                           .signalSemaphoreCount = 0,
                           .pSignalSemaphores = nullptr };

        vkQueueSubmit(device_->get_graphics_queue(), 1, &info, VK_NULL_HANDLE);
        vkQueueWaitIdle(device_->get_graphics_queue());
    }

    VkCommandBuffer* CommandBuffer::get_handle_address() {
        return &command_buffer_;
    }

}  // namespace njin::vulkan
