#pragma once
#include "vulkan/LogicalDevice.h"
#include "vulkan/Swapchain.h"

namespace njin::vulkan {
    class RenderPass {
        public:
        RenderPass(const LogicalDevice& device,
                   const VkRenderPassCreateInfo2& create_info);

        RenderPass(const RenderPass& other) = delete;

        RenderPass& operator=(const RenderPass& other) = delete;

        RenderPass(RenderPass&& other) = delete;

        RenderPass& operator=(RenderPass&& other) = delete;

        ~RenderPass();

        VkRenderPass get() const;

        private:
        const VkDevice device_;
        VkRenderPass render_pass_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
