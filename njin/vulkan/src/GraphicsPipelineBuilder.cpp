#include "vulkan/GraphicsPipelineBuilder.h"

namespace njin::vulkan {

    GraphicsPipelineBuilder::GraphicsPipelineBuilder(
    const LogicalDevice& device,
    const RenderPass& render_pass,
    const PipelineLayout& pipeline_layout) :
        device_{ device },
        render_pass_{ render_pass.get() },
        pipeline_layout_{ pipeline_layout.get() } {}

    void GraphicsPipelineBuilder::add_shader_stage(const ShaderModule& shader) {
        VkPipelineShaderStageCreateInfo shader_stage_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = shader.get_stage(),
            .module = shader.get(),
            .pName = shader_entry_point_.c_str(),
            .pSpecializationInfo = nullptr
        };
        stages_.push_back(shader_stage_info);
    }

    void GraphicsPipelineBuilder::add_vertex_input_binding_description(
    const VkVertexInputBindingDescription& binding_description) {
        vertex_binding_descriptions_.push_back(binding_description);

        vertex_input_info_.vertexBindingDescriptionCount =
        static_cast<uint32_t>(vertex_binding_descriptions_.size());
        vertex_input_info_.pVertexBindingDescriptions =
        vertex_binding_descriptions_.data();
    }

    void GraphicsPipelineBuilder::add_vertex_attribute_description(
    VkVertexInputAttributeDescription attribute_description) {
        vertex_attribute_descriptions_.push_back(attribute_description);

        vertex_input_info_.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertex_attribute_descriptions_.size());
        vertex_input_info_.pVertexAttributeDescriptions =
        vertex_attribute_descriptions_.data();
    }

    void GraphicsPipelineBuilder::set_primitive_topology(VkPrimitiveTopology
                                                         topology) {
        input_assembly_info_.topology = topology;
    }

    void GraphicsPipelineBuilder::set_rasterization_state(
    const RasterizationSettings& settings) {
        rasterization_info_.polygonMode = settings.polygon_mode;
        rasterization_info_.cullMode = settings.cull_mode;
        rasterization_info_.frontFace = settings.front_face;
    }

    void GraphicsPipelineBuilder::set_dynamic_state(
    const std::vector<VkDynamicState>& dynamic_states) {
        dynamic_states_ = dynamic_states;
        dynamic_state_info_.dynamicStateCount =
        static_cast<uint32_t>(dynamic_states.size());
        dynamic_state_info_.pDynamicStates = dynamic_states_.data();
    }

    GraphicsPipeline GraphicsPipelineBuilder::build() {
        // update color blend state info
        VkPipelineColorBlendAttachmentState color_blend_attachment_state{};
        color_blend_attachment_state.blendEnable = VK_FALSE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                                      VK_COLOR_COMPONENT_G_BIT |
                                                      VK_COLOR_COMPONENT_B_BIT |
                                                      VK_COLOR_COMPONENT_A_BIT;

        color_blend_attachment_states_.push_back(color_blend_attachment_state);
        color_blend_info_.attachmentCount =
        color_blend_attachment_states_.size();
        color_blend_info_.pAttachments = color_blend_attachment_states_.data();

        // finally, build the pipeline
        VkGraphicsPipelineCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(stages_.size()),
            .pStages = stages_.data(),
            .pVertexInputState = &vertex_input_info_,
            .pInputAssemblyState = &input_assembly_info_,
            .pTessellationState = &tessellation_info_,
            .pViewportState = &viewport_info_,
            .pRasterizationState = &rasterization_info_,
            .pMultisampleState = &multisample_info_,
            .pDepthStencilState = &depth_stencil_info_,
            .pColorBlendState = &color_blend_info_,
            .pDynamicState = &dynamic_state_info_,
            .layout = pipeline_layout_,
            .renderPass = render_pass_,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1
        };

        return { device_, info };
    }
}  // namespace njin::vulkan