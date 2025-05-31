#include "vulkan/RenderPass.h"

#include <stdexcept>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "vulkan/Swapchain.h"

namespace {
    using namespace njin::vulkan;

    /**
     * Make a VkRenderPassCreateInfo
     * @param attachment_descriptions Attachment description list
     * @param subpass_descriptions Subpass description list
     * @param subpass_dependencies
     * @return VkRenderPassCreateInfo
     */
    VkRenderPassCreateInfo2 make_render_pass_create_info(
    const std::vector<VkAttachmentDescription2>& attachment_descriptions,
    const std::vector<VkSubpassDescription2>& subpass_descriptions,
    const std::vector<VkSubpassDependency2>& subpass_dependencies);

    VkRenderPassCreateInfo2 make_render_pass_create_info(
    const std::vector<VkAttachmentDescription2>& attachment_descriptions,
    const std::vector<VkSubpassDescription2>& subpass_descriptions,
    const std::vector<VkSubpassDependency2>& subpass_dependencies) {
        VkRenderPassCreateInfo2 info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
        info.pNext = nullptr;
        info.flags = 0;
        info.attachmentCount = attachment_descriptions.size();
        info.pAttachments = attachment_descriptions.data();
        info.subpassCount = subpass_descriptions.size();
        info.pSubpasses = subpass_descriptions.data();
        info.dependencyCount = subpass_dependencies.size();
        info.pDependencies = subpass_dependencies.data();
        info.correlatedViewMaskCount = 0;
        info.pCorrelatedViewMasks = nullptr;

        return info;
    }
}  // namespace

namespace njin::vulkan {

    RenderPass::RenderPass(const LogicalDevice& device,
                           const RenderPassInfo& render_pass_info,
                           const AttachmentImages& attachment_images) :
        device_{ device.get() } {
        // create the VkAttachmentDescriptions
        std::vector<AttachmentDescription> attachment_descriptions{
            render_pass_info.attachment_list
        };

        std::vector<VkAttachmentDescription> vk_attachment_descriptions{};

        // attachments subpasses use are specified via an integer index into
        // the attachments array of the parent render pass, so we keep a record
        // of what index an attachment's name maps to
        int counter{ 0 };
        std::unordered_map<std::string, uint32_t> attachment_indices{};
        for (const AttachmentDescription& attachment_description :
             attachment_descriptions) {
            VkImageCreateInfo image_create_info{
                attachment_images
                .get_image_create_info(attachment_description.attachment_name)
            };
            VkAttachmentDescription description{
                .flags = 0,
                .format = image_create_info.format,
                .samples = image_create_info.samples,
                .loadOp = attachment_description.load_op,
                .storeOp = attachment_description.store_op,
                .stencilLoadOp = attachment_description.stencil_load_op,
                .stencilStoreOp = attachment_description.stencil_store_op,
                .initialLayout = attachment_description.initial_layout,
                .finalLayout = attachment_description.final_layout
            };

            vk_attachment_descriptions.push_back(description);

            // record index of attachment name
            attachment_indices[attachment_description.attachment_name] =
            counter;
            ++counter;

            // record attachment names
            attachment_names_.push_back(attachment_description.attachment_name);
        }

        // make the subpass descriptions
        int subpass_counter{ 0 };
        std::vector<SubpassInfo> subpass_infos{ render_pass_info.subpasses };
        std::vector<VkSubpassDescription> subpass_descriptions{};
        for (const SubpassInfo& subpass_info : subpass_infos) {
            // record the index of this subpass
            subpass_index_[subpass_info.name] = subpass_counter;
            ++subpass_counter;

            // record the name of this subpass
            subpasses_.push_back(subpass_info.name);

            int this_color{ color_next_available_ };
            int this_depth{ depth_next_available_ };
            std::vector<std::string> attachment_names{};
            color_attachment_references_.push_back({});
            // make attachment references for color attachments
            for (const AttachmentReference& reference :
                 subpass_info.color_attachments) {
                VkAttachmentReference attachment_reference{
                    .attachment = attachment_indices[reference.attachment_name],
                    .layout = reference.layout
                };
                color_attachment_references_[this_color]
                .push_back(attachment_reference);
                ++color_next_available_;
                attachment_names.push_back(reference.attachment_name);
            }

            subpass_color_attachment_index_[subpass_info.name] =
            attachment_names;

            // depth attachment reference (there should only be one)
            for (const AttachmentReference& reference :
                 subpass_info.depth_attachments) {
                VkAttachmentReference depth_attachment_reference{
                    .attachment = attachment_indices[reference.attachment_name],
                    .layout = reference.layout
                };
                depth_attachment_references_
                .push_back(depth_attachment_reference);
                ++depth_next_available_;
            }

            VkSubpassDescription description{
                .flags = 0,
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0,
                .pInputAttachments = nullptr,
                .colorAttachmentCount = static_cast<uint32_t>(
                color_attachment_references_[this_color].size()),
                .pColorAttachments =
                color_attachment_references_[this_color].data(),
                .pResolveAttachments = nullptr,
                .pDepthStencilAttachment =
                &depth_attachment_references_[this_depth],
                .preserveAttachmentCount = 0,
                .pPreserveAttachments = nullptr
            };

            subpass_descriptions.push_back(description);
        }

        VkRenderPassCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount =
            static_cast<uint32_t>(vk_attachment_descriptions.size()),
            .pAttachments = vk_attachment_descriptions.data(),
            .subpassCount = static_cast<uint32_t>(subpass_descriptions.size()),
            .pSubpasses = subpass_descriptions.data(),
            .dependencyCount = 0,
            .pDependencies = nullptr
        };

        if (vkCreateRenderPass(device_, &create_info, nullptr, &render_pass_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    RenderPass::RenderPass(const LogicalDevice& device,
                           const VkRenderPassCreateInfo2& create_info) :
        device_{ device.get() } {
        if (vkCreateRenderPass2(device_,
                                &create_info,
                                nullptr,
                                &render_pass_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    RenderPass::RenderPass(RenderPass&& other) noexcept :
        name_{ other.name_ },
        subpass_color_attachment_index_{
            other.subpass_color_attachment_index_
        },
        attachment_names_{ other.attachment_names_ },
        subpasses_{ other.subpasses_ },
        subpass_index_{ other.subpass_index_ },
        color_next_available_{ other.color_next_available_ },
        color_attachment_references_{ other.color_attachment_references_ },
        depth_next_available_{ other.depth_next_available_ },
        depth_attachment_references_{ other.depth_attachment_references_ },
        device_{ other.device_ },
        render_pass_{ other.render_pass_ } {
        other.device_ = VK_NULL_HANDLE;
        other.render_pass_ = VK_NULL_HANDLE;
    }

    RenderPass& RenderPass::operator=(RenderPass&& other) noexcept {
        name_ = other.name_;
        subpass_color_attachment_index_ = other.subpass_color_attachment_index_;
        attachment_names_ = other.attachment_names_;
        subpasses_ = other.subpasses_;
        subpass_index_ = other.subpass_index_;
        color_next_available_ = other.color_next_available_;
        color_attachment_references_ = other.color_attachment_references_;
        depth_next_available_ = other.depth_next_available_;
        depth_attachment_references_ = other.depth_attachment_references_;
        device_ = other.device_;
        render_pass_ = other.render_pass_;
        other.device_ = VK_NULL_HANDLE;
        other.render_pass_ = VK_NULL_HANDLE;

        return *this;
    }

    RenderPass::~RenderPass() {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyRenderPass(device_, render_pass_, nullptr);
        }
    }

    std::vector<std::string>
    RenderPass::get_subpass_color_attachment_list(const std::string& name)
    const {
        return subpass_color_attachment_index_.at(name);
    }

    VkRenderPass RenderPass::get() const {
        return render_pass_;
    }

    std::vector<std::string> RenderPass::get_attachment_list() const {
        return attachment_names_;
    }

    uint32_t RenderPass::get_subpass_index(const std::string& name) const {
        return subpass_index_.at(name);
    }

    std::vector<std::string> RenderPass::get_subpass_names() const {
        return subpasses_;
    }
}  // namespace njin::vulkan
