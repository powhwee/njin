#include "vulkan/RenderPassBuilder.h"

#include <stdexcept>

namespace njin::vulkan {

    RenderPassBuilder::RenderPassBuilder(const LogicalDevice& device) :
        device_{ device } {}

    void RenderPassBuilder::add_attachment_description(
    const AttachmentId& attachment_id,
    const AttachmentDescription& attachment_description) {
        VkAttachmentDescription2 desc{
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2,
            .pNext = nullptr,
            .flags = attachment_description.flags,
            .format = attachment_description.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = attachment_description.load_op,
            .storeOp = attachment_description.store_op,
            .stencilLoadOp = attachment_description.stencil_load_op,
            .stencilStoreOp = attachment_description.stencil_store_op,
            .initialLayout = attachment_description.initial_layout,
            .finalLayout = attachment_description.final_layout
        };

        attachment_descriptions_.push_back(desc);
        attachment_map_
        .insert({ attachment_id,
                  static_cast<uint32_t>(attachment_map_.size()) });
    }

    void RenderPassBuilder::add_subpass_description(const SubpassId& subpass_id,
                                                    const SubpassDescription&
                                                    subpass_description) {
        /** process input attachments */
        uint32_t input_attachment_count{ 0 };
        for (const auto& [id, reference_info] :
             subpass_description.input_attachments) {
            ++input_attachment_count;
            if (!attachment_map_.contains(id)) {
                throw std::runtime_error(
                "no attachment with that id currently exists");
            }

            // make the attachment reference
            const uint32_t index{ attachment_map_.at(id) };
            VkAttachmentReference2 ref{
                .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                .pNext = nullptr,
                .attachment = index,
                .layout = reference_info.layout,
                .aspectMask = reference_info.aspect_mask
            };
            // push attachment reference one by one into storage
            attachment_references_.push_front(ref);
        }
        // input attachments can be retrieved in order by iterating from here
        auto input_attachments_start{ attachment_references_.begin() };

        /** process color attachments */
        uint32_t color_attachment_count{ 0 };
        for (const auto& [id, reference_info] :
             subpass_description.color_attachments) {
            ++color_attachment_count;
            if (!attachment_map_.contains(id)) {
                throw std::runtime_error(
                "no attachment with that id currently exists");
            }

            // make the attachment reference
            const uint32_t index{ attachment_map_.at(id) };
            VkAttachmentReference2 ref{
                .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
                .pNext = nullptr,
                .attachment = index,
                .layout = reference_info.layout,
                .aspectMask = reference_info.aspect_mask
            };

            // push attachment reference one by one into storage
            attachment_references_.push_front(ref);
        }
        auto color_attachments_start{ attachment_references_.begin() };

        /** process depth attachment */
        auto [depth_id, depth_info] = subpass_description.depth_attachment;
        if (!attachment_map_.contains(depth_id)) {
            throw std::runtime_error(
            "no attachment with that id currently exists");
        }
        const uint32_t depth_attachment_index{ attachment_map_.at(depth_id) };
        VkAttachmentReference2 depth{
            .sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
            .pNext = nullptr,
            .attachment = depth_attachment_index,
            .layout = depth_info.layout,
            .aspectMask = depth_info.aspect_mask
        };
        attachment_references_.push_front(depth);
        auto depth_attachment{ attachment_references_.begin() };

        // defer the VkSubpassDescription2 building till ::build() because
        // that struct needs pointers to contiguous memory
        // we don't know how many more subpass descriptions will be added
        // so we don't have stable pointers to contiguous memory yet
        // (vector invalidates when resizing, list is stable but not contiguous)
        SubpassDescriptionInfo subpass_description_info{
            .index = static_cast<uint32_t>(subpass_descriptions_.size()),
            .color_attachment_count = color_attachment_count,
            .color_attachments_start = color_attachments_start,
            .input_attachment_count = input_attachment_count,
            .input_attachments_start = input_attachments_start,
            .depth_attachment = depth_attachment,
        };

        subpass_map_.insert({ subpass_id, subpass_description_info });
        subpass_descriptions_.push_back(subpass_description_info);
    }

    void RenderPassBuilder::add_subpass_dependency(const SubpassDependency&
                                                   subpass_dependency) {
        if (!subpass_map_.contains(subpass_dependency.src_id)) {
            throw std::runtime_error(
            "src subpass with given id does not exist");
        }
        if (!subpass_map_.contains(subpass_dependency.dst_id)) {
            throw std::runtime_error(
            "dst subpass with given id does not exist");
        }

        SubpassDescriptionInfo src_subpass_info{
            subpass_map_.at(subpass_dependency.src_id)
        };
        SubpassDescriptionInfo dst_subpass_info{
            subpass_map_.at(subpass_dependency.dst_id)
        };

        VkSubpassDependency2 dependency{
            .sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2,
            .pNext = nullptr,
            .srcSubpass = src_subpass_info.index,
            .dstSubpass = dst_subpass_info.index,
            .srcStageMask = subpass_dependency.src_stage_mask,
            .dstStageMask = subpass_dependency.dst_stage_mask,
            .srcAccessMask = subpass_dependency.src_access_mask,
            .dstAccessMask = subpass_dependency.dst_access_mask,
            .dependencyFlags = 0,
            .viewOffset = 0
        };

        subpass_dependencies_.push_back(dependency);
    }

    RenderPass RenderPassBuilder::build() {
        /** retrieve all the attachments into a contiguous array */
        std::vector<VkAttachmentDescription2> attachment_descriptions{};
        for (auto it{ attachment_descriptions_.begin() };
             it != attachment_descriptions_.end();
             ++it) {
            attachment_descriptions.emplace_back(*it);
        }

        /** make the subpass descriptions array */
        std::vector<VkAttachmentReference2> attachment_references{};
        for (auto it{ attachment_references_.begin() };
             it != attachment_references_.end();
             ++it) {
            attachment_references.emplace_back(*it);
        }

        // find the desired pointer into the attachment references array, from
        // an iterator
        auto find_pointer =
        [this,
         &attachment_references](std::list<VkAttachmentReference2>::iterator it)
        -> VkAttachmentReference2* {
            // this would happen if there were zero of a certain type of attachment
            // added; the iterator into the std::list pointing to newly added
            // attachments would have been set to .end()
            if (it == attachment_references_.end()) {
                return nullptr;
            }

            int count{ 0 };
            while (it != attachment_references_.begin()) {
                --it;
                ++count;
            }

            return &attachment_references.at(count);
        };

        std::vector<VkSubpassDescription2> subpass_descriptions{};
        for (const auto& subpass_description_info : subpass_descriptions_) {
            VkSubpassDescription2 subpass_description{
                .sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2,
                .pNext = nullptr,
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .viewMask = 0,
                .inputAttachmentCount =
                subpass_description_info.input_attachment_count,
                .pInputAttachments =
                find_pointer(subpass_description_info.input_attachments_start),
                .colorAttachmentCount =
                subpass_description_info.color_attachment_count,
                .pColorAttachments =
                find_pointer(subpass_description_info.color_attachments_start),
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment =
                find_pointer(subpass_description_info.depth_attachment),
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr
            };

            subpass_descriptions.push_back(subpass_description);
        }

        // subpass dependencies are already in a contiguous array
        VkRenderPassCreateInfo2 info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount =
            static_cast<uint32_t>(attachment_descriptions.size()),
            .pAttachments = attachment_descriptions.data(),
            .subpassCount = static_cast<uint32_t>(subpass_descriptions.size()),
            .pSubpasses = subpass_descriptions.data(),
            .dependencyCount =
            static_cast<uint32_t>(subpass_dependencies_.size()),
            .pDependencies = subpass_dependencies_.data(),
            .correlatedViewMaskCount = 0,
            .pCorrelatedViewMasks = nullptr
        };

        return { device_, info };
    }
}  // namespace njin::vulkan
