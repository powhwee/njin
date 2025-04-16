#include "vulkan/FramebufferSets.h"

#include "vulkan/RenderPasses.h"

namespace njin::vulkan {
    FramebufferSets::FramebufferSets(const LogicalDevice& device,
                                     const RenderPasses& render_passes,
                                     AttachmentImages& attachment_images) {
        std::vector<std::string> render_pass_names{
            render_passes.get_render_pass_names()
        };

        for (const std::string& name : render_pass_names) {
            FramebufferSet set{ device,
                                render_passes.get_render_pass(name),
                                attachment_images };
            framebuffer_sets_[name] = std::move(set);
        }
    }

    const FramebufferSet*
    FramebufferSets::get_framebuffer_set(const std::string& render_pass_name) {
        return &framebuffer_sets_[render_pass_name];
    }
}  // namespace njin::vulkan