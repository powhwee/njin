#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "RenderPass.h"

namespace njin::vulkan {

    using AttachmentId = std::string;
    using SubpassId = std::string;

    /**
     * Data for constructing a VkAttachmentDescription2
     */
    struct AttachmentDescription {
        VkAttachmentDescriptionFlags flags;
        VkFormat format;
        VkAttachmentLoadOp load_op;
        VkAttachmentStoreOp store_op;
        VkAttachmentLoadOp stencil_load_op;
        VkAttachmentStoreOp stencil_store_op;
        VkImageLayout initial_layout;
        VkImageLayout final_layout;
    };

    /**
     * Data for constructing a VkAttachmentReference2
     */
    struct AttachmentReference {
        VkImageLayout layout;
        VkImageAspectFlags aspect_mask;
    };

    /**
     * Data for constructing a VkSubpassDescription2
     */
    struct SubpassDescription {
        VkSubpassDescriptionFlags flags;
        std::vector<std::pair<AttachmentId, AttachmentReference>>
        input_attachments;
        std::vector<std::pair<AttachmentId, AttachmentReference>>
        color_attachments;
        std::pair<AttachmentId, AttachmentReference> depth_attachment;
    };

    /**
     * Data for constructing a VkSubpassDependency2
     */
    struct SubpassDependency {
        SubpassId dst_id;
        SubpassId src_id;
        VkPipelineStageFlags src_stage_mask;
        VkPipelineStageFlags dst_stage_mask;
        VkAccessFlags src_access_mask;
        VkAccessFlags dst_access_mask;
    };

    /**
    * Builder for VkRenderPasses
    */
    class RenderPassBuilder {
        public:
        RenderPassBuilder(const LogicalDevice& device);

        void add_attachment_description(const AttachmentId& attachment_id,
                                        const AttachmentDescription&
                                        attachment_description);

        void
        add_subpass_description(const SubpassId& subpass_id,
                                const SubpassDescription& subpass_description);

        void
        add_subpass_dependency(const SubpassDependency& subpass_dependency);

        RenderPass build();

        private:
        // location of subpass attachment info in private member std::lists
        struct SubpassDescriptionInfo {
            uint32_t index;
            uint32_t color_attachment_count;  // number of color attachment refs
            std::list<VkAttachmentReference2>::iterator color_attachments_start;
            uint32_t input_attachment_count;  // number of input attachment refs
            std::list<VkAttachmentReference2>::iterator input_attachments_start;
            std::list<VkAttachmentReference2>::iterator depth_attachment;
        };

        // maps AttachmentId to index in attachment descriptions array
        std::unordered_map<AttachmentId, uint32_t> attachment_map_{};
        std::unordered_map<SubpassId, SubpassDescriptionInfo> subpass_map_{};
        std::list<VkAttachmentDescription2> attachment_descriptions_{};
        std::vector<SubpassDescriptionInfo> subpass_descriptions_{};
        std::list<VkAttachmentReference2> attachment_references_{};
        std::vector<VkSubpassDependency2> subpass_dependencies_{};

        const LogicalDevice& device_;
    };
}  // namespace njin::vulkan