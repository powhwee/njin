#pragma once
#include <variant>
#include <vector>

#include <vulkan/vulkan.h>

#include "math/njMat4.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    /**
   * Set 0
   * Binding 0: Model transform matrices for each entity
   * Binding 1: View and projection matrices for the camera. This is independent
   * of each entity
   */
    constexpr int MAX_OBJECTS = 100;

    struct DESCRIPTOR_SET_LAYOUT_BINDING_MODEL_FORMAT {
        math::njMat4f model;
    };

    using MainDrawModel = DESCRIPTOR_SET_LAYOUT_BINDING_MODEL_FORMAT;

    inline SetLayoutBindingInfo DESCRIPTOR_SET_LAYOUT_BINDING_MODEL{
        .name = "model",
        .binding = { .binding = 0,
                     .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                     .descriptorCount = MAX_OBJECTS,
                     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                     .pImmutableSamplers = nullptr },
        .extra_info =
        SetLayoutBindingBufferInfo{
            .size = sizeof(DESCRIPTOR_SET_LAYOUT_BINDING_MODEL_FORMAT) }
    };

    struct DESCRIPTOR_SET_LAYOUT_BINDING_VIEW_PROJECTION_FORMAT {
        math::njMat4f view;
        math::njMat4f projection;
    };

    using MainDrawViewProjection =
    DESCRIPTOR_SET_LAYOUT_BINDING_VIEW_PROJECTION_FORMAT;

    inline SetLayoutBindingInfo DESCRIPTOR_SET_LAYOUT_BINDING_VIEW_PROJECTION{
        .name = "view_projection",
        .binding = { .binding = 1,
                     .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                     .descriptorCount = 1,
                     .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                     .pImmutableSamplers = nullptr },
        .extra_info =
        SetLayoutBindingBufferInfo{
            .size =
            sizeof(DESCRIPTOR_SET_LAYOUT_BINDING_VIEW_PROJECTION_FORMAT) }
    };

    inline SetLayoutInfo DESCRIPTOR_SET_LAYOUT_INFO_MVP{
        "mvp",
        { DESCRIPTOR_SET_LAYOUT_BINDING_MODEL,
          DESCRIPTOR_SET_LAYOUT_BINDING_VIEW_PROJECTION }
    };

    /**
     * Configurations for images to be used as framebuffer attachments.
     * The imageview used for presentation to the screen is omitted, because
     * it is created automatically by the swapchain
     */
    inline AttachmentImageInfo ATTACHMENT_IMAGE_INFO_DEPTH{
        .name = "depth",
        .flags = 0,
        .image_type = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_D32_SFLOAT,
        .mip_levels = 1,
        .array_layers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    };

    /** Graphics pipeline configurations */

    /**
     * Main drawing pipeline
     */
    // 4 bytes for the integer index into the model matrix descriptor array
    inline VkPushConstantRange PUSH_CONSTANT_RANGE_MAIN_DRAW_MODEL{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = 4
    };

    inline ShaderStageInfo SHADER_STAGE_INFO_MAIN_DRAW_VERTEX{
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .shader_path = SHADER_DIR "/shader.vert"
    };

    inline ShaderStageInfo SHADER_STAGE_INFO_MAIN_DRAW_FRAGMENT{
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .shader_path = SHADER_DIR "/shader.frag"
    };
    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_POSITION{
        .name = "position",
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0
    };

    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_NORMAL{
        .name = "normal",
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 12
    };
    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_TANGENT{
        .name = "tangent",
        .location = 2,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = 24
    };
    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_TEX_COORD{
        .name = "tex_coord",
        .location = 3,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 40
    };
    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_COLOR{
        .name = "color",
        .location = 4,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = 48
    };

    // data format for the vertex input of the main render pass - draw subpass
    // 4 + 4 + 4 = 12B vertex
    struct VERTEX_INPUT_MAIN_DRAW_FORMAT {
        float x;
        float y;
        float z;
    };

    using MainDrawVertex = VERTEX_INPUT_MAIN_DRAW_FORMAT;

    inline VertexInputInfo VERTEX_INPUT_INFO_MAIN_DRAW{
        .name = "vertex",
        .vertex_size = 12,
        .attribute_infos = { VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_POSITION }
    };
    inline InputAssemblyInfo INPUT_ASSEMBLY_INFO_MAIN_DRAW{
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    inline RasterizationInfo RASTERIZATION_INFO_MAIN_DRAW{
        .polygon_mode = VK_POLYGON_MODE_LINE,
        .cull_mode = VK_CULL_MODE_BACK_BIT,
        .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .line_width = 1.f
    };

    inline DepthStencilInfo DEPTH_STENCIL_INFO_MAIN_DRAW{
        .depth_test_enable = VK_TRUE,
        .depth_write_enable = VK_TRUE,
        .depth_compare_op = VK_COMPARE_OP_LESS,
    };

    // there is only one color attachment in the "draw" subpass
    inline AttachmentColorBlendInfo ATTACHMENT_COLOR_BLEND_INFO_MAIN_DRAW_COLOR{
        .attachment_name = "swapchain",
        .blend_enable = VK_FALSE,
        .color_write_mask = VK_COLOR_COMPONENT_R_BIT |
                            VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };
    inline ColorBlendInfo COLOR_BLEND_INFO_MAIN_DRAW{
        .logic_op_enable = VK_FALSE,
        .attachment_blend_infos = { ATTACHMENT_COLOR_BLEND_INFO_MAIN_DRAW_COLOR },
    };

    inline DynamicStateInfo DYNAMIC_STATE_INFO_MAIN_DRAW{
        .dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT,
                            VK_DYNAMIC_STATE_SCISSOR }
    };

    inline PipelineInfo PIPELINE_INFO_MAIN_DRAW{
        .render_pass = "main",
        .subpass = "draw",
        .set_layouts = { "mvp" },
        .push_constant_ranges = { PUSH_CONSTANT_RANGE_MAIN_DRAW_MODEL },
        .shader_stages = { SHADER_STAGE_INFO_MAIN_DRAW_VERTEX,
                           SHADER_STAGE_INFO_MAIN_DRAW_FRAGMENT },
        .vertex_input = VERTEX_INPUT_INFO_MAIN_DRAW,
        .input_assembly = INPUT_ASSEMBLY_INFO_MAIN_DRAW,
        .rasterization = RASTERIZATION_INFO_MAIN_DRAW,
        .depth_stencil = DEPTH_STENCIL_INFO_MAIN_DRAW,
        .color_blend = COLOR_BLEND_INFO_MAIN_DRAW,
        .dynamic = DYNAMIC_STATE_INFO_MAIN_DRAW
    };

    /**
     * Render pass configurations
     */

    /**
     * Main render pass
     */
    inline VertexBufferInfo VERTEX_BUFFER_INFO_MAIN_DRAW{
        .name = "main_draw",
        .vertex_input = VERTEX_INPUT_INFO_MAIN_DRAW,
        .max_vertex_count = 1000
    };
    inline AttachmentReference ATTACHMENT_REFERENCE_MAIN_DRAW_SWAPCHAIN{
        .attachment_name = "swapchain",
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    inline AttachmentReference ATTACHMENT_REFERENCE_MAIN_DRAW_DEPTH{
        .attachment_name = "depth",
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };
    inline SubpassInfo SUBPASS_INFO_MAIN_DRAW{
        .name = "draw",
        .pipeline = PIPELINE_INFO_MAIN_DRAW,
        .vertex_buffer = VERTEX_BUFFER_INFO_MAIN_DRAW,
        .color_attachments = { ATTACHMENT_REFERENCE_MAIN_DRAW_SWAPCHAIN },
        .depth_attachments = { ATTACHMENT_REFERENCE_MAIN_DRAW_DEPTH }
    };

    inline AttachmentDescription
    RENDER_PASS_MAIN_ATTACHMENT_DESCRIPTION_SWAPCHAIN{
        .attachment_name = "swapchain",
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    inline AttachmentDescription RENDER_PASS_MAIN_ATTACHMENT_DESCRIPTION_DEPTH{
        .attachment_name = "depth",
        .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    inline RenderPassInfo RENDER_PASS_INFO_MAIN{
        .name = "main",
        .attachment_list = { RENDER_PASS_MAIN_ATTACHMENT_DESCRIPTION_SWAPCHAIN,
                             RENDER_PASS_MAIN_ATTACHMENT_DESCRIPTION_DEPTH },
        .subpasses = { SUBPASS_INFO_MAIN_DRAW }
    };

    /**
     * Debug render pass
     */
    // inline AttachmentReference ATTACHMENT_REFERENCE_DEBUG_DRAW_COLOR{
    //     .attachment_name = "swapchain",
    //     .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    // };
    //
    // inline AttachmentReference ATTACHMENT_REFERENCE_DEBUG_DRAW_DEPTH{
    //     .attachment_name = "depth",
    //     .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
    // };
    //
    // TODO
    // inline SubpassInfo SUBPASS_INFO_DEBUG_DRAW{
    //     .name = "draw",
    //     .pipeline = PIPELINE_INFO_DEBUG_DRAW,
    //     .vertex_buffer = VERTEX_BUFFER
    //     .color_attachments = { ATTACHMENT_REFERENCE_MAIN_DRAW_SWAPCHAIN },
    //     .depth_attachments = { ATTACHMENT_REFERENCE_MAIN_DRAW_DEPTH }
    // };
    //
    // inline AttachmentDescription
    // RENDER_PASS_DEBUG_ATTACHMENT_DESCRIPTION_SWAPCHAIN{
    //     .attachment_name = "swapchain",
    //     .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
    //     .store_op = VK_ATTACHMENT_STORE_OP_STORE,
    //     .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    //     .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    //     .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    // };
    //
    // inline AttachmentDescription RENDER_PASS_DEBUG_ATTACHMENT_DESCRIPTION_DEPTH{
    //     .attachment_name = "depth",
    //     .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
    //     .store_op = VK_ATTACHMENT_STORE_OP_STORE,
    //     .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    //     .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    //     .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
    //     .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    // };
    //
    // inline RenderPassInfo RENDER_PASS_INFO_DEBUG{
    //     .name = "debug",
    //     .attachment_list = { RENDER_PASS_DEBUG_ATTACHMENT_DESCRIPTION_SWAPCHAIN,
    //                          RENDER_PASS_DEBUG_ATTACHMENT_DESCRIPTION_DEPTH },
    //     .subpasses = { SUBPASS_INFO_DEBUG_DRAW }

    inline RendererInfo RENDERER_INFO{ .render_passes = {
                                           RENDER_PASS_INFO_MAIN } };
};  // namespace njin::vulkan
