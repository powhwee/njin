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
                           const VkRenderPassCreateInfo2& create_info) :
        device_{ device.get() } {
        if (vkCreateRenderPass2(device_,
                                &create_info,
                                nullptr,
                                &render_pass_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    RenderPass::~RenderPass() {
        vkDestroyRenderPass(device_, render_pass_, nullptr);
    }

    VkRenderPass RenderPass::get() const {
        return render_pass_;
    }
}  // namespace njin::vulkan
