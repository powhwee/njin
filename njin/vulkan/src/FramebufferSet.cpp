#include "vulkan/FramebufferSet.h"

#include <stdexcept>

#include "vulkan/ImageBuilder.h"

namespace njin::vulkan {
    FramebufferSet::FramebufferSet(const LogicalDevice& device,
                                   const RenderPass& render_pass,
                                   const std::vector<ImageView*>& image_views) :
        device_{ device.get() },
        extent_{ image_views[0]->get_extent() } {
        // make the framebuffer
        VkFramebufferCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.renderPass = render_pass.get();

        std::vector<VkImageView> views{};
        for (const auto& image_view : image_views) {
            views.push_back(image_view->get());
        }

        info.attachmentCount = views.size();
        info.pAttachments = views.data();
        info.width = image_views[0]->get_extent().width;
        info.height = image_views[0]->get_extent().height;
        info.layers = 1;

        if (vkCreateFramebuffer(device_, &info, nullptr, &framebuffers_[0]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        };
    }

    FramebufferSet::FramebufferSet(const LogicalDevice& device,
                                   const RenderPass& render_pass,
                                   AttachmentImages& images) :
        device_{ device.get() },
        extent_{ images.get_extent() } {
        std::vector<std::string> attachment_names{
            render_pass.get_attachment_list()
        };
        std::vector<std::vector<VkImageView>> attachment_sets{
            images.make_image_view_sets(attachment_names)
        };

        size_t attachment_set_count{ attachment_sets.size() };
        framebuffers_.resize(attachment_set_count);

        for (int i{ 0 }; i < attachment_set_count; ++i) {
            VkFramebufferCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .renderPass = render_pass.get(),
                .attachmentCount = static_cast<uint32_t>(attachment_set_count),
                .pAttachments = attachment_sets[i].data(),
                .width = images.get_extent().width,
                .height = images.get_extent().height,
                .layers = 1
            };
            if (vkCreateFramebuffer(device_,
                                    &info,
                                    nullptr,
                                    &framebuffers_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer");
            }
        }
    }

    FramebufferSet::FramebufferSet(FramebufferSet&& other) noexcept :
        device_{ other.device_ },
        framebuffers_{ other.framebuffers_ },
        extent_{ other.extent_ } {
        other.device_ = nullptr;
        other.framebuffers_ = {};
        other.extent_ = {};
    }

    FramebufferSet& FramebufferSet::operator=(FramebufferSet&& other) noexcept {
        device_ = other.device_;
        framebuffers_ = other.framebuffers_;
        extent_ = other.extent_;
        other.device_ = nullptr;
        other.framebuffers_ = {};
        other.extent_ = {};
        return *this;
    }

    FramebufferSet::~FramebufferSet() {
        if (device_ != nullptr) {
            for (VkFramebuffer framebuffer : framebuffers_) {
                vkDestroyFramebuffer(device_, framebuffer, nullptr);
            }
        }
    }

    VkFramebuffer FramebufferSet::get_framebuffer(uint32_t index) const {
        return framebuffers_[index];
    }

    std::vector<VkFramebuffer> FramebufferSet::get() const {
        return framebuffers_;
    }

    VkExtent2D FramebufferSet::get_extent() const {
        return extent_;
    };
}  // namespace njin::vulkan
