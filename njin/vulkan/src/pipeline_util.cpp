#include "vulkan/pipeline_util.h"

#include "vulkan/ShaderModule.h"

namespace njin::vulkan {
    VkPipelineShaderStageCreateInfo
    make_shader_stage_create_info(const ShaderModule& module) {
        VkPipelineShaderStageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.stage = module.get_stage();
        info.module = module.get();
        info.pName = module.get_entry_point();
        info.pSpecializationInfo = nullptr;

        return info;
    }

    VkPipelineInputAssemblyStateCreateInfo
    make_input_assembly_state_create_info(VkPrimitiveTopology topology) {
        VkPipelineInputAssemblyStateCreateInfo info{};
        info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.topology = topology;
        info.primitiveRestartEnable = VK_FALSE;

        return info;
    }

    VkPipelineTessellationStateCreateInfo
    make_tessellation_state_create_info() {
        VkPipelineTessellationStateCreateInfo info{};
        // TODO
        return info;
    }

    VkPipelineViewportStateCreateInfo make_viewport_state_create_info() {
        VkPipelineViewportStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.viewportCount = 1;
        info.pViewports = nullptr;  // dynamic
        info.scissorCount = 1;
        info.pScissors = nullptr;  // dynamic

        return info;
    }

    VkPipelineRasterizationStateCreateInfo
    make_rasterization_state_create_info() {
        VkPipelineRasterizationStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.depthClampEnable = VK_FALSE;
        info.rasterizerDiscardEnable = VK_FALSE;
        info.polygonMode = VK_POLYGON_MODE_FILL;
        info.cullMode = VK_CULL_MODE_BACK_BIT;
        info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        info.depthBiasEnable = VK_FALSE;
        info.depthBiasConstantFactor = 0.0f;
        info.depthBiasClamp = 0.0f;
        info.lineWidth = 1.0f;

        return info;
    }

    VkPipelineMultisampleStateCreateInfo make_multisample_state_create_info() {
        VkPipelineMultisampleStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        info.sampleShadingEnable = VK_FALSE;
        info.minSampleShading = 1.0f;
        info.pSampleMask = nullptr;
        info.alphaToCoverageEnable = VK_FALSE;
        info.alphaToOneEnable = VK_FALSE;

        return info;
    }

    VkPipelineDepthStencilStateCreateInfo
    make_depth_stencil_state_create_info() {
        VkPipelineDepthStencilStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.depthTestEnable = VK_TRUE;
        info.depthWriteEnable = VK_TRUE;
        info.depthCompareOp = VK_COMPARE_OP_LESS;
        info.depthBoundsTestEnable = VK_FALSE;
        info.stencilTestEnable = VK_FALSE;
        info.front = {};
        info.back = {};
        info.minDepthBounds = 0.0f;
        info.maxDepthBounds = 1.0f;

        return info;
    }

    VkPipelineColorBlendStateCreateInfo make_color_blend_state_create_info(
    const std::vector<VkPipelineColorBlendAttachmentState>& attachments) {
        VkPipelineColorBlendStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.logicOpEnable = VK_FALSE;
        info.logicOp = VK_LOGIC_OP_COPY;
        info.attachmentCount = attachments.size();
        info.pAttachments = attachments.data();
        info.blendConstants[0] = 0.0f;
        info.blendConstants[1] = 0.0f;
        info.blendConstants[2] = 0.0f;
        info.blendConstants[3] = 0.0f;

        return info;
    }

    VkPipelineColorBlendAttachmentState make_color_blend_attachment_state() {
        VkPipelineColorBlendAttachmentState info{};
        info.blendEnable = VK_FALSE;
        info.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        info.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        info.colorBlendOp = VK_BLEND_OP_ADD;
        info.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        info.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        info.alphaBlendOp = VK_BLEND_OP_ADD;
        info.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                              VK_COLOR_COMPONENT_G_BIT |
                              VK_COLOR_COMPONENT_B_BIT |
                              VK_COLOR_COMPONENT_A_BIT;

        return info;
    }

    VkPipelineDynamicStateCreateInfo
    make_dynamic_state_create_info(const std::vector<VkDynamicState>&
                                   dynamic_states) {
        VkPipelineDynamicStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.dynamicStateCount = dynamic_states.size();
        info.pDynamicStates = dynamic_states.data();

        return info;
    }

    VkPipelineVertexInputStateCreateInfo make_vertex_input_state_create_info(
    const std::vector<VkVertexInputBindingDescription>& bindings,
    const std::vector<VkVertexInputAttributeDescription>& attributes) {
        VkPipelineVertexInputStateCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.vertexBindingDescriptionCount = bindings.size();
        info.pVertexBindingDescriptions = bindings.data();
        info.vertexAttributeDescriptionCount = attributes.size();
        info.pVertexAttributeDescriptions = attributes.data();

        return info;
    }
}  // namespace njin::vulkan
