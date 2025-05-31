#pragma once
#include <string>
#include <unordered_map>

#include "GraphicsPipeline.h"
#include "RenderPasses.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/RenderPass.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {

    /**
     * Registry for all graphics pipelines
     */
    class GraphicsPipelines {
        public:
        GraphicsPipelines(const LogicalDevice& device,
                          const RenderPasses& render_passes,
                          const DescriptorSets& descriptor_sets,
                          const std::vector<PipelineInfo>& pipeline_infos);

        VkPipeline get_graphics_pipeline(const std::string& render_pass_name,
                                         const std::string& subpass_name) const;

        VkPipelineLayout
        get_pipeline_layout(const std::string& render_pass_name,
                            const std::string& subpass_name) const;

        private:
        struct Hash {
            template<typename T1, typename T2>
            std::size_t operator()(const std::pair<T1, T2>& pair) const {
                return std::hash<T1>{}(pair.first) ^
                       (std::hash<T2>{}(pair.second) << 1);
            }
        };

        std::unordered_map<std::pair<std::string, std::string>,
                           GraphicsPipeline,
                           Hash>
        graphics_pipelines_;
    };
}  // namespace njin::vulkan