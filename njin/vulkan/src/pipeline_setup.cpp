#include "vulkan/pipeline_setup.h"

#include "core/njVertex.h"
#include "vulkan/DescriptorSetLayoutBuilder.h"

constexpr int MAX_DEBUG_OBJECTS = 100;

namespace njin::vulkan {

    GraphicsPipeline
    make_debug_pipeline(const LogicalDevice& logical_device,
                        const RenderPass& render_pass,
                        const PipelineLayout& pipeline_layout) {
        GraphicsPipelineBuilder builder{ logical_device,
                                         render_pass,
                                         pipeline_layout };
        /** shader modules */
        ShaderModule vertex_shader_module{ logical_device,
                                           SHADER_DIR "/debug.vert",
                                           VK_SHADER_STAGE_VERTEX_BIT };
        ShaderModule fragment_shader_module{ logical_device,
                                             SHADER_DIR "/debug.frag",
                                             VK_SHADER_STAGE_FRAGMENT_BIT };
        builder.add_shader_stage(vertex_shader_module);
        builder.add_shader_stage(fragment_shader_module);

        /** vertex input */
        builder.add_vertex_input_binding_description(
        core::njVertex::get_binding_description());

        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{
            core::njVertex::get_attribute_descriptions()
        };

        for (const auto& description : attribute_descriptions) {
            builder.add_vertex_attribute_description(description);
        }

        builder.set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

        GraphicsPipelineBuilder::RasterizationSettings rasterization_settings{
            .polygon_mode = VK_POLYGON_MODE_FILL,
            .cull_mode = VK_CULL_MODE_NONE,
            .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        };

        builder.set_rasterization_state(rasterization_settings);

        std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT,
                                                    VK_DYNAMIC_STATE_SCISSOR };
        builder.set_dynamic_state(dynamic_states);

        return builder.build();
    }
}  // namespace njin::vulkan