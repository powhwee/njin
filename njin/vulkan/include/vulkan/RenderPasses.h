#pragma once
#include <string>
#include <unordered_map>

#include "RenderPass.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    /**
     * Registry for all render passes
     */
    class RenderPasses {
        public:
        RenderPasses(const LogicalDevice& device,
                     const std::vector<RenderPassInfo>& infos,
                     const AttachmentImages& attachment_images);

        /**
          * Gets a render pass by its name
          * @param name Name of the render pass to retrieve
          * @return Reference to the requested render pass
          * @throws std::out_of_range if the render pass doesn't exist
          */
        const RenderPass& get_render_pass(const std::string& name) const {
            return render_passes_.at(name);
        }

        std::vector<std::string> get_render_pass_names() const;

        private:
        std::vector<std::string> names_;
        std::unordered_map<std::string, RenderPass> render_passes_;
    };
}  // namespace njin::vulkan
