#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan/GraphicsPipeline.h"
#include "vulkan/ShaderModule.h"

namespace njin::vulkan {

    class GraphicsPipelineBuilder {
        public:
        struct RasterizationSettings {
            VkPolygonMode polygon_mode;
            VkCullModeFlags cull_mode;
            VkFrontFace front_face;
        };

        GraphicsPipelineBuilder(const LogicalDevice& device,
                                const RenderPass& render_pass,
                                const PipelineLayout& pipeline_layout);

        void add_shader_stage(const ShaderModule& shader);

        void add_vertex_input_binding_description(
        const VkVertexInputBindingDescription& binding_description);

        void add_vertex_attribute_description(VkVertexInputAttributeDescription
                                              attribute_description);

        void set_primitive_topology(VkPrimitiveTopology topology);

        void set_rasterization_state(const RasterizationSettings& settings);

        void
        set_dynamic_state(const std::vector<VkDynamicState>& dynamic_states);

        GraphicsPipeline build();

        private:
        /** shader stages */
        const std::string shader_entry_point_{ "main" };
        std::vector<VkPipelineShaderStageCreateInfo> stages_{};

        /** vertex input state */
        std::vector<VkVertexInputBindingDescription>
        vertex_binding_descriptions_{};

        std::vector<VkVertexInputAttributeDescription>
        vertex_attribute_descriptions_{};

        VkPipelineVertexInputStateCreateInfo vertex_input_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 0,
            .pVertexBindingDescriptions = vertex_binding_descriptions_.data(),
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions =
            vertex_attribute_descriptions_.data()
        };

        /** input assembly state info */
        VkPipelineInputAssemblyStateCreateInfo input_assembly_info_{
            .sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        /** tessellation state */
        VkPipelineTessellationStateCreateInfo tessellation_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .patchControlPoints = 1
        };

        /** viewport state */
        VkPipelineViewportStateCreateInfo viewport_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,
            .scissorCount = 1,
            .pScissors = nullptr
        };

        /** rasterization state */
        VkPipelineRasterizationStateCreateInfo rasterization_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = 0,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.f,
            .depthBiasClamp = 0.f,
            .depthBiasSlopeFactor = 0.f,
            .lineWidth = 1.f
        };

        /** multisample state */
        VkPipelineMultisampleStateCreateInfo multisample_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 0,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        /** depth stencil state */

        VkPipelineDepthStencilStateCreateInfo depth_stencil_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.0f,
            .maxDepthBounds = 1.0f
        };

        /** color blend state */
        std::vector<VkPipelineColorBlendAttachmentState>
        color_blend_attachment_states_{};

        VkPipelineColorBlendStateCreateInfo color_blend_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = VK_FALSE,
            .logicOp = VK_LOGIC_OP_COPY,
            .attachmentCount =
            static_cast<uint32_t>(color_blend_attachment_states_.size()),
            .pAttachments = color_blend_attachment_states_.data(),
            //             .blendConstants[0] = 0.f,
            //             .blendConstants[1] = 0.f,
            //             .blendConstants[2] = 0.f,
            //             .blendConstants[3] = 0.f
        };

        /** dynamic state */
        std::vector<VkDynamicState> dynamic_states_{};
        VkPipelineDynamicStateCreateInfo dynamic_state_info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount = 0,
            .pDynamicStates = dynamic_states_.data(),
        };

        /** other stuff */
        const LogicalDevice& device_;
        VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
        VkRenderPass render_pass_{ VK_NULL_HANDLE };
    };

}  // namespace njin::vulkan
