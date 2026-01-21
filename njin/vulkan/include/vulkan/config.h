#pragma once
#include <variant>
#include <vector>

#include <vulkan/vulkan.h>

#include "math/njMat4.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    /**
     * Descriptor set configurations
     */

    /**
   * MVP descriptor set
   * Binding 0: Model transform matrices for each entity
   * Binding 1: View and projection matrices for the camera. This is independent
   * of each entity
   */
    constexpr int MAX_OBJECTS = 16;

    struct DESCRIPTOR_SET_LAYOUT_BINDING_MODEL_FORMAT {
        math::njMat4f model;
    };

    using MainDrawModel = DESCRIPTOR_SET_LAYOUT_BINDING_MODEL_FORMAT;

    inline SetLayoutBindingInfo DESCRIPTOR_SET_LAYOUT_BINDING_MODEL{
        .name = "model",
        .binding = 0,
        .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptor_count = 16,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
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
        .binding = 1,
        .descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptor_count = 16,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
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
     * Texture images descriptor set
     * Binding 0: Texture images
     */

    inline SetLayoutBindingInfo DESCRIPTOR_SET_LAYOUT_BINDING_IMAGE{
        .name = "image",
        .binding = 0,
        .descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptor_count = 16,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .extra_info =
        SetLayoutBindingImageInfo{
            .sampler_info = { .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                              .pNext = nullptr,
                              .flags = 0,
                              .magFilter = VK_FILTER_LINEAR,
                              .minFilter = VK_FILTER_LINEAR,
                              .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
                              .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                              .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                              .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                              .mipLodBias = 0,
                              .anisotropyEnable = VK_FALSE,
                              .maxAnisotropy = 0,
                              .compareEnable = VK_FALSE,
                              .minLod = 0.f,
                              .maxLod = 0.f,
                              .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                              .unnormalizedCoordinates = VK_FALSE } }
    };

    inline SetLayoutInfo DESCRIPTOR_SET_LAYOUT_TEXTURES{
        .name = "textures",
        .binding_infos = { DESCRIPTOR_SET_LAYOUT_BINDING_IMAGE }
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

    // 8 bytes for model_index (4) + texture_index (4)
    inline VkPushConstantRange PUSH_CONSTANT_RANGE_MAIN_DRAW_MODEL{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = 8
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
        .format = VK_FORMAT_R16G16B16A16_UNORM,
        .offset = 48
    };

    // data format for the vertex input of the main render pass - draw subpass
    struct VERTEX_INPUT_MAIN_DRAW_FORMAT {
        float x, y, z;
        float nx, ny, nz;
        float tx, ty, tz, tw;
        float u, v;
        uint16_t r, g, b, a;
    };

    using MainDrawVertex = VERTEX_INPUT_MAIN_DRAW_FORMAT;

    inline VertexInputInfo VERTEX_INPUT_INFO_MAIN_DRAW{
        .name = "vertex",
        .vertex_size = 56,
        .attribute_infos = { VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_POSITION,
                             VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_NORMAL,
                             VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_TANGENT,
                             VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_TEX_COORD,
                             VERTEX_ATTRIBUTE_INFO_MAIN_DRAW_COLOR }
    };
    inline InputAssemblyInfo INPUT_ASSEMBLY_INFO_MAIN_DRAW{
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    inline RasterizationInfo RASTERIZATION_INFO_MAIN_DRAW{
        .polygon_mode = VK_POLYGON_MODE_FILL,
        .cull_mode = VK_CULL_MODE_NONE,
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
        .set_layouts = { "mvp", "textures" },  // Added textures for fragment shader
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
     * Isometric sprite pipeline
     *
     */

    // 4 bytes for the integer index into the model matrix descriptor array
    inline VkPushConstantRange PUSH_CONSTANT_RANGE_ISO_DRAW_MODEL{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = 4
    };

    inline VkPushConstantRange PUSH_CONSTANT_RANGE_ISO_DRAW_TEXTURE{
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 4,
        .size = 4
    };

    inline ShaderStageInfo SHADER_STAGE_INFO_ISO_DRAW_VERTEX{
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .shader_path = SHADER_DIR "/iso.vert"
    };

    inline ShaderStageInfo SHADER_STAGE_INFO_ISO_DRAW_FRAGMENT{
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .shader_path = SHADER_DIR "/iso.frag"
    };

    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_ISO_DRAW_POSITION{
        .name = "position",
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0
    };

    inline VertexAttributeInfo VERTEX_ATTRIBUTE_INFO_ISO_DRAW_UV{
        .name = "uv",
        .location = 1,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 12
    };

    inline VertexInputInfo VERTEX_INPUT_INFO_ISO_DRAW{
        .name = "vertex",
        .vertex_size = 20,
        .attribute_infos = { VERTEX_ATTRIBUTE_INFO_ISO_DRAW_POSITION,
                             VERTEX_ATTRIBUTE_INFO_ISO_DRAW_UV }
    };

    struct VERTEX_INPUT_ISO_DRAW_FORMAT {
        // position
        float x;
        float y;
        float z;

        // uv
        float u;
        float v;
    };

    using IsoDrawVertex = VERTEX_INPUT_ISO_DRAW_FORMAT;

    inline InputAssemblyInfo INPUT_ASSEMBLY_INFO_ISO_DRAW{
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    inline RasterizationInfo RASTERIZATION_INFO_ISO_DRAW{
        .polygon_mode = VK_POLYGON_MODE_FILL,
        .cull_mode = VK_CULL_MODE_BACK_BIT,
        .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .line_width = 1.f
    };

    inline DepthStencilInfo DEPTH_STENCIL_INFO_ISO_DRAW{
        .depth_test_enable = VK_TRUE,
        .depth_write_enable = VK_FALSE,
        .depth_compare_op = VK_COMPARE_OP_LESS
    };

    inline AttachmentColorBlendInfo ATTACHMENT_COLOR_BLEND_INFO_ISO_DRAW_COLOR{
        .attachment_name = "swapchain",
        .blend_enable = VK_TRUE,
        .src_color_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dst_color_blend_factor = VK_BLEND_FACTOR_DST_ALPHA,
        .color_blend_op = VK_BLEND_OP_ADD,
        .src_alpha_blend_factor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dst_alpha_blend_factor = VK_BLEND_FACTOR_DST_ALPHA,
        .alpha_blend_op = VK_BLEND_OP_ADD,
        .color_write_mask = VK_COLOR_COMPONENT_R_BIT |
                            VK_COLOR_COMPONENT_G_BIT |
                            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    inline ColorBlendInfo COLOR_BLEND_INFO_ISO_DRAW{
        .logic_op_enable = VK_FALSE,
        .attachment_blend_infos = { ATTACHMENT_COLOR_BLEND_INFO_ISO_DRAW_COLOR }
    };

    inline DynamicStateInfo DYNAMIC_STATE_INFO_ISO_DRAW{
        .dynamic_states = { VK_DYNAMIC_STATE_VIEWPORT,
                            VK_DYNAMIC_STATE_SCISSOR }
    };

    inline PipelineInfo PIPELINE_INFO_ISO_DRAW{
        .render_pass = "iso",
        .subpass = "draw",
        .set_layouts = { "mvp", "textures" },
        .push_constant_ranges = { PUSH_CONSTANT_RANGE_ISO_DRAW_MODEL,
                                  PUSH_CONSTANT_RANGE_ISO_DRAW_TEXTURE },
        .shader_stages = { SHADER_STAGE_INFO_ISO_DRAW_VERTEX,
                           SHADER_STAGE_INFO_ISO_DRAW_FRAGMENT },
        .vertex_input = VERTEX_INPUT_INFO_ISO_DRAW,
        .input_assembly = INPUT_ASSEMBLY_INFO_ISO_DRAW,
        .rasterization = RASTERIZATION_INFO_ISO_DRAW,
        .depth_stencil = DEPTH_STENCIL_INFO_ISO_DRAW,
        .color_blend = COLOR_BLEND_INFO_ISO_DRAW,
        .dynamic = DYNAMIC_STATE_INFO_ISO_DRAW
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
        .max_vertex_count = 100000
    };

    inline IndexBufferInfo INDEX_BUFFER_INFO_MAIN_DRAW{
        .name = "main_draw",
        .max_index_count = 300000
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
        .index_buffer_name = "main_draw",
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
        .final_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
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
    * Isometric sprite drawing render pass
    */
    inline VertexBufferInfo VERTEX_BUFFER_INFO_ISO_DRAW{
        .name = "iso_draw",
        .vertex_input = VERTEX_INPUT_INFO_ISO_DRAW,
        .max_vertex_count = 1000
    };

    inline AttachmentReference ATTACHMENT_REFERENCE_ISO_DRAW_SWAPCHAIN{
        .attachment_name = "swapchain",
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    inline AttachmentReference ATTACHMENT_REFERENCE_ISO_DRAW_DEPTH{
        .attachment_name = "depth",
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    inline SubpassInfo SUBPASS_INFO_ISO_DRAW{
        .name = "draw",
        .pipeline = PIPELINE_INFO_ISO_DRAW,
        .vertex_buffer = VERTEX_BUFFER_INFO_ISO_DRAW,
        .color_attachments = { ATTACHMENT_REFERENCE_ISO_DRAW_SWAPCHAIN },
        .depth_attachments = { ATTACHMENT_REFERENCE_ISO_DRAW_DEPTH }
    };

    inline AttachmentDescription
    RENDER_PASS_ISO_ATTACHMENT_DESCRIPTION_SWAPCHAIN{
        .attachment_name = "swapchain",
        .load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    inline AttachmentDescription RENDER_PASS_ISO_ATTACHMENT_DESCRIPTION_DEPTH{
        .attachment_name = "depth",
        .load_op = VK_ATTACHMENT_LOAD_OP_LOAD,
        .store_op = VK_ATTACHMENT_STORE_OP_STORE,
        .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initial_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    inline RenderPassInfo RENDER_PASS_INFO_ISO{
        .name = "iso",
        .attachment_list = { RENDER_PASS_ISO_ATTACHMENT_DESCRIPTION_SWAPCHAIN,
                             RENDER_PASS_ISO_ATTACHMENT_DESCRIPTION_DEPTH },
        .subpasses = { SUBPASS_INFO_ISO_DRAW }
    };

    /**
     * Overall renderer configuration
     */
    inline RendererInfo RENDERER_INFO{
        .render_passes = { RENDER_PASS_INFO_MAIN }  // Removed iso pass
    };
};  // namespace njin::vulkan
