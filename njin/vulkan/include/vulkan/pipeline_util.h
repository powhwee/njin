#pragma once
#include <vulkan/vulkan_core.h>
#include <string>
#include <vector>

#include "ShaderModule.h"

/**
* Graphics pipeline creation involves a lot of *CreateInfo setup -
* all the helper functions for that are located here
*/

namespace njin::vulkan {
    /**
     * Helper method for creating VkPipelineShaderStageCreateInfo
     * @param module Shader module to use in this stage
     * @return VkPipelineShaderStageCreateInfo
     * @note Assumes that the name of the entry point for a shader module is
     * "main"
     */
    VkPipelineShaderStageCreateInfo make_shader_stage_create_info(
        const ShaderModule& module);

    /**
     * Helper method for creating VkPipelineVertexInputStateCreateInfo
     * @return VkPipelineVertexInputStateCreateInfo
     */
    VkPipelineVertexInputStateCreateInfo make_vertex_input_state_create_info(
        const std::vector<VkVertexInputBindingDescription>& bindings,
        const std::vector<VkVertexInputAttributeDescription>& attributes);


    /**
     * Helper method for creating VkPipelineInputAssemblyStateCreateInfo
     * @param topology Primitive topology type
     * @return VkPipelineInputAssemblyStateCreateInfo
     */
    VkPipelineInputAssemblyStateCreateInfo
    make_input_assembly_state_create_info(VkPrimitiveTopology topology);

    /**
     * Helper method for creating VkPipelineTessellationStateCreateInfo
     * @return VkPipelineTessellationStateCreateInfo
     */
    VkPipelineTessellationStateCreateInfo
    make_tessellation_state_create_info();

    /**
     * Helper method for creating VkPipelineViewportStateCreateInfo
     * @return VkPipelineViewportStateCreateInfo
     */
    VkPipelineViewportStateCreateInfo
    make_viewport_state_create_info();

    /**
     * Helper method for creating VkPipelineRasterizationStateCreateInfo
     * @return VkPipelineRasterizationStateCreateInfo
     */
    VkPipelineRasterizationStateCreateInfo
    make_rasterization_state_create_info();

    /**
     * Helper method for creating VkPipelineMultisampleStateCreateInfo
     * @return VkPipelineMultisampleStateCreateInfo
     */
    VkPipelineMultisampleStateCreateInfo
    make_multisample_state_create_info();

    /**
     * Helper method for creating VkPipelineDepthStencilStateCreateInfo
     * @return VkPipelineDepthStencilStateCreateInfo
     */
    VkPipelineDepthStencilStateCreateInfo
    make_depth_stencil_state_create_info();

    /**
     * Helper method for creating VkPipelineColorBlendStateCreateInfo
     * @param attachments Color blend state attachments
     * @return VkPipelineColorBlendStateCreateInfo
     */
    VkPipelineColorBlendStateCreateInfo
    make_color_blend_state_create_info(
        const std::vector<VkPipelineColorBlendAttachmentState>& attachments);

    /**
     * Make a VkPipelineColorBlendAttachmentState
     * @return VkPipelineColorBlendAttachmentState
     */
    VkPipelineColorBlendAttachmentState make_color_blend_attachment_state();

    /**
     * Make a VkPipelineDynamicStateCreateInfo
     * @param dynamic_states List of dynamic states to use
     * @return VkPipelineDynamicStateCreateInfo
     */
    VkPipelineDynamicStateCreateInfo make_dynamic_state_create_info(
        const std::vector<VkDynamicState>& dynamic_states);
}
