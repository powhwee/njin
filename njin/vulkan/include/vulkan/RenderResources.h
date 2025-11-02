#pragma once
#include "CommandPool.h"
#include "FramebufferSets.h"
#include "vulkan/DescriptorSets.h"
#include "vulkan/GraphicsPipeline.h"
#include "vulkan/GraphicsPipelines.h"
#include "vulkan/RenderPass.h"
#include "vulkan/RenderPasses.h"
#include "vulkan/VertexBuffers.h"
#include "vulkan/IndexBuffers.h"

namespace njin::vulkan {

    /**
     * Contains all information about the resources to create
     */
    struct RenderResourceInfos {
        std::vector<AttachmentImageInfo> attachment_images;
        std::vector<SetLayoutInfo> set_layouts;
        std::vector<RenderPassInfo> render_passes;
        std::vector<PipelineInfo> pipelines;
        std::vector<VertexBufferInfo> vertex_buffers;
        std::vector<IndexBufferInfo> index_buffers;
    };

    /**
     * Registry for all the resources involved in rendering:
     *     - images to be used as attachments
     *     - framebuffers
     *     - vertex buffers
     *     - descriptor sets
     *     - render passes
     *     - graphics pipelines
     */
    class RenderResources {
        public:
        RenderResources(const LogicalDevice& logical_device,
                        const PhysicalDevice& physical_device,
                        Swapchain& swapchain,
                        const RenderResourceInfos& infos);

        CommandPool& get_command_pool();

        DescriptorSets descriptor_sets;
        AttachmentImages attachment_images;
        RenderPasses render_passes;
        GraphicsPipelines graphics_pipelines;
        VertexBuffers vertex_buffers;
        IndexBuffers index_buffers;
        FramebufferSets framebuffers;

        // for renderer so that it can acquire command buffers for drawing etc
        CommandPool command_pool_;
    };

}  // namespace njin::vulkan