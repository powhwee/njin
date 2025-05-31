#pragma once
#include "AttachmentImages.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/Swapchain.h"

namespace njin::vulkan {
    class RenderPass {
        public:
        RenderPass(const LogicalDevice& device,
                   const RenderPassInfo& render_pass_info,
                   const AttachmentImages& attachment_images);
        RenderPass(const LogicalDevice& device,
                   const VkRenderPassCreateInfo2& create_info);

        RenderPass(const RenderPass& other) = delete;

        RenderPass& operator=(const RenderPass& other) = delete;

        RenderPass(RenderPass&& other) noexcept;

        RenderPass& operator=(RenderPass&& other) noexcept;

        ~RenderPass();

        /**
         * Retrieve the list of attachment names a subpass in this
         * render pass is using
         * @param name Name of subpass
         * @return Attachment name list
         */
        std::vector<std::string>
        get_subpass_color_attachment_list(const std::string& name) const;
        VkRenderPass get() const;

        /**
         * Get the ordered name list of attachments this render pass requires
         * @return Ordered name list of attachments
         */
        std::vector<std::string> get_attachment_list() const;

        uint32_t get_subpass_index(const std::string& name) const;

        std::vector<std::string> get_subpass_names() const;

        private:
        std::string name_;
        std::unordered_map<std::string, std::vector<std::string>>
        subpass_color_attachment_index_;

        std::vector<std::string> attachment_names_{};

        // name list of subpasses in this render pass
        std::vector<std::string> subpasses_;

        std::unordered_map<std::string, uint32_t> subpass_index_;

        // one vector of color attachments for each subpass
        int color_next_available_{ 0 };
        std::vector<std::vector<VkAttachmentReference>>
        color_attachment_references_{};

        // one depth attachment for each subpass
        int depth_next_available_{ 0 };
        std::vector<VkAttachmentReference> depth_attachment_references_{};
        VkDevice device_;
        VkRenderPass render_pass_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
