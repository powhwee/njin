#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan/LogicalDevice.h"
#include "vulkan/RenderPass.h"

namespace njin::vulkan {
    class Framebuffer {
        public:
        /**
         * Constructor
         * @param device Device to create this framebuffer on
         * @param render_pass Render pass this framebuffer will be compatible with
         * @param image_views Array of image views to be used as attachments
         * @note Ordering of the image views in image_views must match
         * the order as described during the creation of the render pass
         */
        Framebuffer(const LogicalDevice& device,
                    const RenderPass& render_pass,
                    const std::vector<ImageView*>& image_views);

        Framebuffer(const Framebuffer& other) = delete;

        Framebuffer& operator=(const Framebuffer& other) = delete;

        Framebuffer(Framebuffer&& other) noexcept;

        Framebuffer& operator=(Framebuffer&& other) noexcept;

        ~Framebuffer();

        VkFramebuffer get() const;

        VkExtent2D get_extent() const;

        private:
        VkDevice device_{ VK_NULL_HANDLE };
        VkFramebuffer framebuffer_{ VK_NULL_HANDLE };

        // extent of the attachment in this framebuffer
        VkExtent2D extent_;
    };
}  // namespace njin::vulkan
