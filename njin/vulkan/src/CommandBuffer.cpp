#include "vulkan/CommandBuffer.h"

#include <stdexcept>

#include <vulkan/vulkan_core.h>

#include "vulkan/GraphicsPipeline.h"

namespace njin::vulkan {
    CommandBuffer::CommandBuffer(const LogicalDevice& device,
                                 VkCommandBuffer command_buffer) :
        device_{ &device },
        command_buffer_{ command_buffer } {}

    VkCommandBuffer CommandBuffer::get() const {
        return command_buffer_;
    }

    void CommandBuffer::begin(VkCommandBufferUsageFlags usage,
                              VkCommandBufferInheritanceInfo inheritance_info) {
        VkCommandBufferBeginInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = usage,
            .pInheritanceInfo = &inheritance_info
        };

        vkBeginCommandBuffer(command_buffer_, &info);
    }

    void CommandBuffer::end() {
        vkEndCommandBuffer(command_buffer_);
    }

    void CommandBuffer::submit(const CommandBufferSubmitInfo& submit_info) {
        VkSubmitInfo info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount =
            static_cast<uint32_t>(submit_info.wait_semaphores.size()),
            .pWaitSemaphores = submit_info.wait_semaphores.data(),
            .pWaitDstStageMask = submit_info.wait_stages.data(),
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer_,
            .signalSemaphoreCount =
            static_cast<uint32_t>(submit_info.signal_semaphores.size()),
            .pSignalSemaphores = submit_info.signal_semaphores.data(),
        };

        vkQueueSubmit(device_->get_graphics_queue(),
                      1,
                      &info,
                      submit_info.signal_fence);
        if (submit_info.should_wait_idle) {
            vkQueueWaitIdle(device_->get_graphics_queue());
        }
    }

    VkCommandBuffer* CommandBuffer::get_handle_address() {
        return &command_buffer_;
    }

}  // namespace njin::vulkan
