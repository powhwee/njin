#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan/LogicalDevice.h"
#include "vulkan/RenderPass.h"

namespace njin::vulkan {
    class FramebufferSet {
        public:
        FramebufferSet() = default;
        /**
         * Constructor
         * @param device Device to create this framebuffer on
         * @param render_pass Render pass this framebuffer will be compatible with
         * @param image_views Array of image views to be used as attachments
         * @note Ordering of the image views in image_views must match
         * the order as described during the creation of the render pass
         */
        FramebufferSet(const LogicalDevice& device,
                       const RenderPass& render_pass,
                       const std::vector<ImageView*>& image_views);

        /**
         * Constructor
         * @param device Logical device
         * @param render_pass Render pass this framebuffer will be used with
         * @param images Attachment images to form image views from
         */
        FramebufferSet(const LogicalDevice& device,
                       const RenderPass& render_pass,
                       AttachmentImages& images);

        FramebufferSet(const FramebufferSet& other) = delete;

        FramebufferSet& operator=(const FramebufferSet& other) = delete;

        FramebufferSet(FramebufferSet&& other) noexcept;

        FramebufferSet& operator=(FramebufferSet&& other) noexcept;

        ~FramebufferSet();

        VkFramebuffer get_framebuffer(uint32_t index) const;

        std::vector<VkFramebuffer> get() const;

        VkExtent2D get_extent() const;

        private:
        VkDevice device_{ VK_NULL_HANDLE };
        std::vector<VkFramebuffer> framebuffers_{ VK_NULL_HANDLE };

        // extent of the attachment in this framebuffer
        VkExtent2D extent_;
    };
}  // namespace njin::vulkan
