#pragma once
#include "RenderPasses.h"
#include "vulkan/AttachmentImages.h"
#include "vulkan/FramebufferSet.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    class FramebufferSets {
        public:
        FramebufferSets(const LogicalDevice& device,
                        const RenderPasses& render_passes,
                        AttachmentImages& attachment_images);

        const FramebufferSet*
        get_framebuffer_set(const std::string& render_pass_name);

        private:
        std::unordered_map<std::string, FramebufferSet> framebuffer_sets_;
    };

}  // namespace njin::vulkan