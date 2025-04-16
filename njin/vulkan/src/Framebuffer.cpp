#include "vulkan/Framebuffer.h"

#include <stdexcept>

#include "vulkan/ImageBuilder.h"

namespace njin::vulkan {
    Framebuffer::Framebuffer(const LogicalDevice& device,
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

        if (vkCreateFramebuffer(device_, &info, nullptr, &framebuffer_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer");
        };
    }

    Framebuffer::Framebuffer(Framebuffer&& other) noexcept :
        device_{ other.device_ },
        framebuffer_{ other.framebuffer_ },
        extent_{ other.extent_ } {
        other.device_ = nullptr;
        other.framebuffer_ = nullptr;
        other.extent_ = {};
    }

    Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept {
        device_ = other.device_;
        framebuffer_ = other.framebuffer_;
        extent_ = other.extent_;
        other.device_ = nullptr;
        other.framebuffer_ = nullptr;
        other.extent_ = {};
        return *this;
    }

    Framebuffer::~Framebuffer() {
        if (device_ != nullptr) {
            vkDestroyFramebuffer(device_, framebuffer_, nullptr);
        }
    }

    VkFramebuffer Framebuffer::get() const {
        return framebuffer_;
    }

    VkExtent2D Framebuffer::get_extent() const {
        return extent_;
    };
}  // namespace njin::vulkan
