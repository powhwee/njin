#pragma once
#include <variant>

/**
 * All configuration structs used in the setup of Vulkan
 */
namespace njin::vulkan {
    /**
     * Descriptor set configurations
     */
    struct SetLayoutBindingBufferInfo {
        VkDeviceSize size;  // size of individual descriptor
    };

    struct SetLayoutBindingImageInfo {
        VkSamplerCreateInfo sampler_info;
    };

    using SetLayoutBindingExtraInfo =
    std::variant<SetLayoutBindingBufferInfo, SetLayoutBindingImageInfo>;

    struct SetLayoutBindingInfo {
        std::string name;
        uint32_t binding;
        VkDescriptorType descriptor_type;
        uint32_t descriptor_count;
        VkShaderStageFlags stage;
        SetLayoutBindingExtraInfo extra_info;
    };

    struct SetLayoutInfo {
        std::string name;
        std::vector<SetLayoutBindingInfo> binding_infos;
    };

    /**
     * Configuration for images to be used as render pass attachments
     */
    struct AttachmentImageInfo {
        std::string name;
        VkImageCreateFlags flags;
        VkImageType image_type;
        VkFormat format;
        uint32_t mip_levels;
        uint32_t array_layers;
        VkSampleCountFlagBits samples;
        VkImageTiling tiling;
        VkImageUsageFlags usage;
    };

    /**
     * Graphics pipeline configurations
     */
    struct ShaderStageInfo {
        VkShaderStageFlagBits stage;
        std::string shader_path;
    };

    struct VertexAttributeInfo {
        std::string name;
        uint32_t location;
        VkFormat format;
        uint32_t offset;
    };

    struct VertexInputInfo {
        std::string name;
        uint32_t vertex_size;  // total size in bytes of vertex
        std::vector<VertexAttributeInfo> attribute_infos;
    };

    struct InputAssemblyInfo {
        VkPrimitiveTopology topology;
    };

    struct RasterizationInfo {
        VkPolygonMode polygon_mode;
        VkCullModeFlags cull_mode;
        VkFrontFace front_face;
        float line_width;
    };

    struct DepthStencilInfo {
        VkBool32 depth_test_enable;
        VkBool32 depth_write_enable;
        VkCompareOp depth_compare_op;
    };

    /**
     * Controls the blend mode of a particular color attachment bound to
     * the pipeline.
     * @param color_attachment Name of color attachment. Must be an attachment
     * also existing in the subpass the pipeline is used in
     * @param color_write_mask Bitmask of color channels to which writes are permitted
     */
    struct AttachmentColorBlendInfo {
        std::string attachment_name;
        VkBool32 blend_enable;
        VkBlendFactor src_color_blend_factor;
        VkBlendFactor dst_color_blend_factor;
        VkBlendOp color_blend_op;
        VkBlendFactor src_alpha_blend_factor;
        VkBlendFactor dst_alpha_blend_factor;
        VkBlendOp alpha_blend_op;
        VkColorComponentFlags color_write_mask;
    };

    struct BlendConstants {
        float r;
        float g;
        float b;
        float a;
    };

    /**
     * Blend configuration for color attachments bound to the pipeline
     */
    struct ColorBlendInfo {
        VkBool32 logic_op_enable;
        VkLogicOp logic_op;
        std::vector<AttachmentColorBlendInfo> attachment_blend_infos;
        BlendConstants blend_constants;
    };

    struct DynamicStateInfo {
        std::vector<VkDynamicState> dynamic_states;
    };

    /**
     * Configuration struct for a graphics pipeline
     * @param render_pass Name of render pass this graphics pipeline will be used in
     * @param subpass Name of specific subpass within the render pass
     * @param set_layouts Ordered list of descriptor set layouts the pipeline will use
     * @param shader_stages List of ShaderStageInfo for the pipeline's programmable shader stages
     */
    struct PipelineInfo {
        std::string render_pass;
        std::string subpass;
        std::vector<std::string> set_layouts;
        std::vector<VkPushConstantRange> push_constant_ranges;
        std::vector<ShaderStageInfo> shader_stages;
        VertexInputInfo vertex_input;
        InputAssemblyInfo input_assembly;
        RasterizationInfo rasterization;
        DepthStencilInfo depth_stencil;
        ColorBlendInfo color_blend;
        DynamicStateInfo dynamic;
    };

    /**
     * Vertex buffer configurations
     */
    struct VertexBufferInfo {
        std::string name;
        VertexInputInfo vertex_input;
        uint32_t max_vertex_count;
    };

    /**
     * Render pass configuration
     */

    /**
     * Attachment description for render passes.
     * Assumptions:
     * 1. The number of samples in the attachment is the same as the number of
     * samples in the underlying attachment image.
     * 2. The format of the attachment is the same as the format of the
     * underlying attachment image.
     */
    struct AttachmentDescription {
        std::string attachment_name;
        VkAttachmentLoadOp load_op;
        VkAttachmentStoreOp store_op;
        VkAttachmentLoadOp stencil_load_op;
        VkAttachmentStoreOp stencil_store_op;
        VkImageLayout initial_layout;
        VkImageLayout final_layout;
    };

    /**
     * Attachment references for subpasses
     * @param attachment_name Name of attachment in parent render pass,
     * that this subpass will use
     * @param layout Image layout the attachment will transition to for
     * this subpass
     */
    struct AttachmentReference {
        std::string attachment_name;
        VkImageLayout layout;
    };

    /** Configuration struct for a subpass.
     * @param pipeline PipelineInfo of the pipeline this subpass will bind to
     * @param color_attachment_list Ordered namelist of color attachments
     * this subpass will use
     * @param depth_attachment_list Ordered name list of depth attachments
     * this subpass will use
     *
     * @note Names provided must be a subset of those provided in the parent
     * RenderPassInfo
     */
    struct SubpassInfo {
        std::string name;
        PipelineInfo pipeline;
        VertexBufferInfo vertex_buffer;
        std::vector<AttachmentReference> color_attachments;
        std::vector<AttachmentReference> depth_attachments;
    };

    /**
     * Configuration struct for a render pass.
     * @param attachment_list Ordered list of attachment images the render pass will use.
     * Use "swapchain" to refer to the swapchain image.
     * @param subpasses Ordered list of subpasses in this render pass
     */
    struct RenderPassInfo {
        std::string name;
        std::vector<AttachmentDescription> attachment_list;
        std::vector<SubpassInfo> subpasses;
    };

    /**
     * Renderer configuration. Specifies the render pass order
     * @param render_passes Ordered list of render passes
     */
    struct RendererInfo {
        std::vector<RenderPassInfo> render_passes;
    };

}  // namespace njin::vulkan