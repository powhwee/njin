#include "vulkan/RenderPasses.h"

namespace njin::vulkan {

    RenderPasses::RenderPasses(const LogicalDevice& device,
                               const std::vector<RenderPassInfo>& infos,
                               const AttachmentImages& attachment_images) :
        render_passes_{ [&device, &infos, &attachment_images] {
            std::unordered_map<std::string, RenderPass> map;
            for (const RenderPassInfo& info : infos) {
                RenderPass render_pass{ device, info, attachment_images };
                map.emplace(info.name, std::move(render_pass));
            }
            return map;
        }() } {
        // record the names of all the render passes
        for (const RenderPassInfo& info : infos) {
            names_.push_back(info.name);
        }
    }

    std::vector<std::string> RenderPasses::get_render_pass_names() const {
        return names_;
    }
}  // namespace njin::vulkan