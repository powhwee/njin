#include "vulkan/GraphicsPipelines.h"

namespace njin::vulkan {

    GraphicsPipelines::GraphicsPipelines(const LogicalDevice& device,
                                         const RenderPasses& render_passes,
                                         const DescriptorSets& descriptor_sets,
                                         const std::vector<PipelineInfo>&
                                         pipeline_infos) :
        graphics_pipelines_{
            [&device, &render_passes, &descriptor_sets, &pipeline_infos] {
                std::unordered_map<std::pair<std::string, std::string>,
                                   GraphicsPipeline,
                                   Hash>
                map;
                for (const PipelineInfo& pipeline_info : pipeline_infos) {
                    // Create a new graphics pipeline passing the corresponding render pass
                    const RenderPass& render_pass =
                    render_passes.get_render_pass(pipeline_info.render_pass);
                    GraphicsPipeline graphics_pipeline{ device,
                                                        render_pass,
                                                        descriptor_sets,
                                                        pipeline_info };
                    map.emplace(std::make_pair(pipeline_info.render_pass,
                                               pipeline_info.subpass),
                                std::move(graphics_pipeline));
                }
                return map;
            }()
        } {}

    VkPipeline GraphicsPipelines::get_graphics_pipeline(
    const std::string& render_pass_name,
    const std::string& subpass_name) const {
        return graphics_pipelines_.at({ render_pass_name, subpass_name }).get();
    }

    VkPipelineLayout
    GraphicsPipelines::get_pipeline_layout(const std::string& render_pass_name,
                                           const std::string& subpass_name)
    const {
        return graphics_pipelines_.at({ render_pass_name, subpass_name })
        .get_layout();
    }
}  // namespace njin::vulkan