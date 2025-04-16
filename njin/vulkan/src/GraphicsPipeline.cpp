#include "vulkan/GraphicsPipeline.h"

#include <stdexcept>
#include <string>

#include <vulkan/Renderer_DEPRECATE.h>
#include <vulkan/vulkan_core.h>

#include "vulkan/Buffer.h"
#include "vulkan/ShaderModule.h"
#include "vulkan/pipeline_util.h"

namespace {
    using namespace njin::vulkan;
}  // namespace

namespace njin::vulkan {

    GraphicsPipeline::GraphicsPipeline(const LogicalDevice& device,
                                       const VkGraphicsPipelineCreateInfo&
                                       info) :
        device_{ device.get() },
        pipeline_layout_{ info.layout } {
        if (vkCreateGraphicsPipelines(device_,
                                      VK_NULL_HANDLE,
                                      1,
                                      &info,
                                      nullptr,
                                      &pipeline_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    GraphicsPipeline::GraphicsPipeline(const LogicalDevice& device,
                                       const RenderPass& render_pass,
                                       const DescriptorSets& descriptor_sets,
                                       const PipelineInfo& pipeline_info) :
        device_{ device.get() } {
        // fill in / derive all the state infos

        // shader stages
        std::vector<VkPipelineShaderStageCreateInfo> shader_stage_infos{};

        // keep the shader modules alive until the pipeline is created
        std::string entry_point{ "main" };
        std::vector<ShaderModule> shader_modules{};
        for (const ShaderStageInfo& stage_info : pipeline_info.shader_stages) {
            ShaderModule shader_module{ device,
                                        stage_info.shader_path,
                                        stage_info.stage };
            VkPipelineShaderStageCreateInfo shader_stage_info{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .stage = stage_info.stage,
                .module = shader_module.get(),
                .pName = entry_point.data(),
                .pSpecializationInfo = nullptr
            };
            shader_stage_infos.push_back(shader_stage_info);
            shader_modules.push_back(std::move(shader_module));
        }

        // vertex input state
        VkVertexInputBindingDescription binding_description{
            .binding = 0,
            .stride = pipeline_info.vertex_input.vertex_size,
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};
        for (const VertexAttributeInfo& info :
             pipeline_info.vertex_input.attribute_infos) {
            VkVertexInputAttributeDescription description{
                .location = info.location,
                .binding = 0,
                .format = info.format,
                .offset = info.offset
            };
            attribute_descriptions.push_back(description);
        }
        VkPipelineVertexInputStateCreateInfo vertex_input_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &binding_description,
            .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(attribute_descriptions.size()),
            .pVertexAttributeDescriptions = attribute_descriptions.data()
        };

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state{
            .sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .topology = pipeline_info.input_assembly.topology,
            .primitiveRestartEnable = VK_FALSE
        };

        VkPipelineTessellationStateCreateInfo tessellation_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .patchControlPoints = 0,
        };

        VkPipelineViewportStateCreateInfo viewport_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .viewportCount = 1,
            .pViewports = nullptr,  // ignored because viewport is dynamic
            .scissorCount = 1,
            .pScissors = nullptr  // ignored because viewport is dynamic
        };

        VkPipelineRasterizationStateCreateInfo rasterization_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthClampEnable = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode = pipeline_info.rasterization.polygon_mode,
            .cullMode = pipeline_info.rasterization.cull_mode,
            .frontFace = pipeline_info.rasterization.front_face,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.f,
            .depthBiasClamp = 0.f,
            .depthBiasSlopeFactor = 0.f,
            .lineWidth = 1.f
        };

        VkPipelineMultisampleStateCreateInfo multisample_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

        VkPipelineDepthStencilStateCreateInfo depth_stencil_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .depthTestEnable = pipeline_info.depth_stencil.depth_test_enable,
            .depthWriteEnable = pipeline_info.depth_stencil.depth_write_enable,
            .depthCompareOp = pipeline_info.depth_stencil.depth_compare_op,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable = VK_FALSE,
            .front = {},
            .back = {},
            .minDepthBounds = 0.f,
            .maxDepthBounds = 1.f
        };

        // Color blend state
        // Each pipeline is meant for use in a specific subpass in a specific
        // render pass. Each subpass picks out attachments it will use from
        // the set of VkAttachmentDescription specified at render pass creation
        // time. The way the subpass picks out attachments is via VkAttachmentReference,
        // which is basically an integer index into the render pass's array of VkAttachmentDescription.
        // Naturally, each attachment used by the subpass will need to have a corresponding
        // color blend attachment state. The documentation is not clear on
        // whether the array of color blend attachment states should match
        // the ordering of VkAttachmentDescriptions, or the ordering
        // of VkAttachmentReferences. The latter is assumed

        // reorder the AttachmentColorBlendInfo to match the AttachmentReference
        // order in the subpass
        std::vector<std::string> attachment_names{
            render_pass.get_subpass_color_attachment_list(pipeline_info.subpass)
        };
        std::vector<VkPipelineColorBlendAttachmentState>
        color_blend_attachment_states{};
        for (const std::string& name : attachment_names) {
            for (const AttachmentColorBlendInfo& blend_info :
                 pipeline_info.color_blend.attachment_blend_infos) {
                if (blend_info.attachment_name == name) {
                    VkPipelineColorBlendAttachmentState
                    color_blend_attachment_state{
                        .blendEnable = blend_info.blend_enable,
                        .srcColorBlendFactor =
                        blend_info.src_color_blend_factor,
                        .dstColorBlendFactor =
                        blend_info.dst_color_blend_factor,
                        .colorBlendOp = blend_info.color_blend_op,
                        .srcAlphaBlendFactor =
                        blend_info.src_alpha_blend_factor,
                        .dstAlphaBlendFactor =
                        blend_info.dst_alpha_blend_factor,
                        .alphaBlendOp = blend_info.alpha_blend_op,
                        .colorWriteMask = blend_info.color_write_mask,
                    };
                    color_blend_attachment_states
                    .push_back(color_blend_attachment_state);
                }
            }
        }

        const auto& [r, g, b, a]{ pipeline_info.color_blend.blend_constants };
        VkPipelineColorBlendStateCreateInfo color_blend_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .logicOpEnable = pipeline_info.color_blend.logic_op_enable,
            .logicOp = pipeline_info.color_blend.logic_op,
            .attachmentCount =
            static_cast<uint32_t>(color_blend_attachment_states.size()),
            .pAttachments = color_blend_attachment_states.data(),
            .blendConstants = { r, g, b, a }
        };

        VkPipelineDynamicStateCreateInfo dynamic_state{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .dynamicStateCount =
            static_cast<uint32_t>(pipeline_info.dynamic.dynamic_states.size()),
            .pDynamicStates = pipeline_info.dynamic.dynamic_states.data(),
        };

        // pipeline layout
        std::vector<VkDescriptorSetLayout> set_layouts{};
        std::vector<std::string> set_layout_names{ pipeline_info.set_layouts };
        for (const std::string& set_layout_name : set_layout_names) {
            set_layouts.push_back(descriptor_sets
                                  .get_descriptor_set_layout(set_layout_name));
        }

        VkPipelineLayoutCreateInfo pipeline_layout_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data(),
            .pushConstantRangeCount =
            static_cast<uint32_t>(pipeline_info.push_constant_ranges.size()),
            .pPushConstantRanges = pipeline_info.push_constant_ranges.data()
        };

        if (vkCreatePipelineLayout(device_,
                                   &pipeline_layout_info,
                                   nullptr,
                                   &pipeline_layout_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create pipeline layout!");
        }

        VkGraphicsPipelineCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stageCount = static_cast<uint32_t>(shader_stage_infos.size()),
            .pStages = shader_stage_infos.data(),
            .pVertexInputState = &vertex_input_state,
            .pInputAssemblyState = &input_assembly_state,
            .pTessellationState = &tessellation_state,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization_state,
            .pMultisampleState = &multisample_state,
            .pDepthStencilState = &depth_stencil_state,
            .pColorBlendState = &color_blend_state,
            .pDynamicState = &dynamic_state,
            .layout = pipeline_layout_,
            .renderPass = render_pass.get(),
            .subpass = render_pass.get_subpass_index(pipeline_info.subpass),
            .basePipelineHandle = nullptr,
            .basePipelineIndex = 0
        };

        if (vkCreateGraphicsPipelines(device_,
                                      VK_NULL_HANDLE,
                                      1,
                                      &info,
                                      nullptr,
                                      &pipeline_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept :
        device_{ other.device_ },
        pipeline_{ other.pipeline_ },
        pipeline_layout_{ other.pipeline_layout_ } {
        other.device_ = VK_NULL_HANDLE;
        other.pipeline_ = VK_NULL_HANDLE;
        other.pipeline_layout_ = VK_NULL_HANDLE;
    }

    GraphicsPipeline::GraphicsPipeline(const LogicalDevice& device,
                                       VkPipeline pipeline) :
        device_{ device.get() },
        pipeline_{ pipeline } {}

    GraphicsPipeline::~GraphicsPipeline() {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
            vkDestroyPipeline(device_, pipeline_, nullptr);
        }
    }

    VkPipeline GraphicsPipeline::get() const {
        return pipeline_;
    }

    VkPipelineLayout GraphicsPipeline::get_layout() const {
        return pipeline_layout_;
    }
}  // namespace njin::vulkan
