#pragma once

#include <core/RenderQueue.h>

#include "Buffer.h"
#include "DescriptorSet.h"
#include "Fence.h"
#include "Semaphore.h"
#include "core/RenderBuffer.h"
#include "core/RenderQueue.h"
#include "core/njVertex.h"
#include "math/njVec3.h"
#include "vulkan/CommandBuffer.h"
#include "vulkan/CommandPool.h"
#include "vulkan/Framebuffer.h"
#include "vulkan/GraphicsPipeline.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/PhysicalDevice.h"
#include "vulkan/RenderPass.h"
#include "vulkan/Swapchain.h"

namespace njin::vulkan {
    /**
    * Coordinator for all the Vulkan components
    * Responsible for using them together to draw frames
    */
    class Renderer {
        public:
        /**
         * Constructor
         * @param physical_device Physical device
         * @param logical_device Logical device
         * @param swapchain Swapchain
         * @param render_pass Render pass to use
         * @param graphics_pipeline Pipeline to use with render pass
         * @param render_buffer Render buffer to read information from
         */
        Renderer(const PhysicalDevice& physical_device,
                 const LogicalDevice& logical_device,
                 Swapchain& swapchain,
                 const RenderPass& render_pass,
                 const GraphicsPipeline& graphics_pipeline,
                 const GraphicsPipeline& debug_pipeline,
                 std::vector<DescriptorSet>& descriptor_sets,
                 const Buffer& vertex_buffer,
                 const Buffer& debug_buffer);

        Renderer(const Renderer& other) = delete;

        Renderer(Renderer&& other) = delete;

        Renderer& operator=(const Renderer& other) = delete;

        Renderer& operator=(Renderer&& other) = delete;

        ~Renderer();

        void draw_frame(core::RenderQueue& render_queue);

        private:
        std::vector<ImageView> depth_image_views_{};
        const PhysicalDevice* physical_device_;
        const LogicalDevice* logical_device_{ nullptr };
        Swapchain& swapchain_;
        const RenderPass* render_pass_;
        const GraphicsPipeline* graphics_pipeline_{ nullptr };
        const GraphicsPipeline* debug_pipeline_{ nullptr };
        std::vector<Framebuffer> framebuffers_{};
        CommandPool command_pool_;
        CommandBuffer command_buffer_;
        VkBuffer vertex_buffer_;
        VkBuffer debug_buffer_;
        std::vector<DescriptorSet*> descriptor_sets_{};

        const core::RenderBuffer* render_buffer_{ nullptr };
        Image depth_image_{};

        // sync objects
        Semaphore image_available_semaphore_;
        Semaphore render_finished_semaphore_;
        Fence in_flight_fence_;

        void init_framebuffers();
    };
}  // namespace njin::vulkan
