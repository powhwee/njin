#include "vulkan/Renderer.h"

#include <iostream>
#include <ostream>
#include <stdexcept>
#include <thread>

#include "vulkan/DescriptorSetLayoutBuilder.h"
#include "vulkan/ImageBuilder.h"
#include "vulkan/ShaderModule.h"
#include "vulkan/pipeline_util.h"

namespace njin::vulkan {

    Renderer::Renderer(const PhysicalDevice& physical_device,
                       const LogicalDevice& logical_device,
                       Swapchain& swapchain,
                       const RenderPass& render_pass,
                       const GraphicsPipeline& graphics_pipeline,
                       const GraphicsPipeline& debug_pipeline,
                       std::vector<DescriptorSet>& descriptor_sets,
                       const Buffer& vertex_buffer,
                       const Buffer& debug_buffer) :
        physical_device_{ &physical_device },
        logical_device_{ &logical_device },
        swapchain_{ swapchain },
        render_pass_{ &render_pass },
        graphics_pipeline_{ &graphics_pipeline },
        debug_pipeline_{ &debug_pipeline },
        command_pool_{ *logical_device_, 0 },
        command_buffer_{ *logical_device_, command_pool_ },
        vertex_buffer_{ vertex_buffer.get() },
        debug_buffer_{ debug_buffer.get() },
        descriptor_sets_{ [&]() -> std::vector<DescriptorSet*> {
            std::vector<DescriptorSet*> sets{};
            for (auto& descriptor_set : descriptor_sets) {
                sets.push_back(&descriptor_set);
            }
            return sets;
        }() },
        image_available_semaphore_{ *logical_device_ },
        render_finished_semaphore_{ *logical_device_ },
        in_flight_fence_{ *logical_device_ } {
        init_framebuffers();
    }

    Renderer::~Renderer() {
        vkDeviceWaitIdle(logical_device_->get());
    }

    void Renderer::draw_frame(core::RenderQueue& render_queue) {
        /** main rendering logic */
        // wait for the last command buffer to tell us it is done executing
        vkWaitForFences(logical_device_->get(),
                        1,
                        in_flight_fence_.get_handle_address(),
                        VK_TRUE,
                        UINT64_MAX);

        vkResetFences(logical_device_->get(),
                      1,
                      in_flight_fence_.get_handle_address());

        // get the index of the next image that will be available
        // NOTE: this does not mean the image can be written to
        // see https://docs.vulkan.org/spec/latest/chapters/VK_KHR_surface/wsi.html#vkAcquireNextImageKHR
        // "The presentation engine may not have finished reading from the image...
        // ...the application must use semaphore and/or fence..."
        uint32_t image_index{ 0 };
        vkAcquireNextImageKHR(logical_device_->get(),
                              swapchain_.get(),
                              UINT64_MAX,
                              image_available_semaphore_.get(),
                              VK_NULL_HANDLE,
                              &image_index);

        // pointer to framebuffer to use
        Framebuffer* framebuffer{ &framebuffers_.at(image_index) };

        VkCommandBuffer command_buffer{ command_buffer_.get() };
        vkResetCommandBuffer(command_buffer,
                             VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
        VkCommandBufferBeginInfo command_buffer_begin_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr
        };

        vkBeginCommandBuffer(command_buffer, &command_buffer_begin_info);

        /** binding resources */

        // descriptor sets
        std::vector<VkDescriptorSet> descriptor_sets{};
        for (const auto& descriptor_set : descriptor_sets_) {
            descriptor_sets.push_back(descriptor_set->get());
        }

        VkClearValue clear_color{ { { 0.0f, 0.0f, 0.0f, 1.0f } } };
        VkOffset2D offset{ 0, 0 };
        VkExtent2D extent{ framebuffer->get_extent() };
        VkClearValue color_clear_value{ .color = { 0.f, 0.f, 0.f, 0.f } };
        VkClearValue depth_clear_value{ .depthStencil = { 1.f, 0 } };
        std::vector<VkClearValue> clear_values{ color_clear_value,
                                                depth_clear_value };

        VkRenderPassBeginInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass_->get(),
            .framebuffer = framebuffer->get(),
            .renderArea = { offset, extent },
            .clearValueCount = static_cast<uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data()
        };
        vkCmdBeginRenderPass(command_buffer,
                             &render_pass_info,
                             VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindDescriptorSets(command_buffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                graphics_pipeline_->get_layout(),
                                0,
                                descriptor_sets.size(),
                                descriptor_sets.data(),
                                0,
                                nullptr);

        vkCmdBindPipeline(command_buffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphics_pipeline_->get());

        // vertex buffer
        VkDeviceSize offsets{ 0 };
        vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer_, &offsets);

        // configure the parts of the pipeline we declared dynamic
        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(framebuffer->get_extent().width),
            .height = static_cast<float>(framebuffer->get_extent().height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        vkCmdSetViewport(command_buffer, 0, 1, &viewport);

        VkRect2D scissor{ .offset = { 0, 0 },
                          .extent = framebuffer->get_extent() };
        vkCmdSetScissor(command_buffer, 0, 1, &scissor);

        // work through the render queue, drawing each object
        while (!render_queue.is_empty()) {
            core::RenderInfo info{ render_queue.pop() };
            // update push constants
            uint32_t values[2] = { info.model_index, info.texture_index };
            // model index
            vkCmdPushConstants(command_buffer,
                               graphics_pipeline_->get_layout(),
                               VK_SHADER_STAGE_VERTEX_BIT,
                               0,
                               sizeof(uint32_t),
                               &values[0]);

            // texture index
            vkCmdPushConstants(command_buffer,
                               graphics_pipeline_->get_layout(),
                               VK_SHADER_STAGE_FRAGMENT_BIT,
                               sizeof(uint32_t),
                               sizeof(uint32_t),
                               &values[1]);

            vkCmdDraw(command_buffer,
                      info.vertex_count,
                      1,
                      info.mesh_offset,
                      0);
        }

        /** draw debug wireframes */
        vkCmdBindPipeline(command_buffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          debug_pipeline_->get());

        vkCmdBindVertexBuffers(command_buffer, 0, 1, &debug_buffer_, &offsets);

        vkCmdDraw(command_buffer, 2, 1, 0, 0);

        vkCmdEndRenderPass(command_buffer);

        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
            throw std::runtime_error(
            "failed to record command buffer commands");
        }

        VkPipelineStageFlags wait_stages[] = {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        VkSubmitInfo submit_info{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = image_available_semaphore_.get_handle_address(),
            .pWaitDstStageMask = wait_stages,
            .commandBufferCount = 1,
            .pCommandBuffers = &command_buffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores =
            render_finished_semaphore_.get_handle_address(),
        };

        if (vkQueueSubmit(logical_device_->get_graphics_queue(),
                          1,
                          &submit_info,
                          in_flight_fence_.get()) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer");
        }

        // present when semaphores signalled (rendering done)
        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = render_finished_semaphore_.get_handle_address(),
            .swapchainCount = 1,
            .pSwapchains = swapchain_.get_handle_address(),
            .pImageIndices = &image_index,
            .pResults = nullptr
        };

        vkQueuePresentKHR(logical_device_->get_present_queue(), &present_info);
    }

    void Renderer::init_framebuffers() {
        // swapchain image views
        std::vector<ImageView*> image_views{ swapchain_.get_image_views() };

        // depth image views
        ImageBuilder depth_image_builder{ *physical_device_, *logical_device_ };
        depth_image_builder.set_image_type(VK_IMAGE_TYPE_2D);
        depth_image_builder.set_format(VK_FORMAT_D32_SFLOAT);
        auto [width, height]{ swapchain_.get_extent() };
        VkExtent3D depth_extent{ width, height, 1 };
        depth_image_builder.set_extent(depth_extent);
        depth_image_builder.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);
        depth_image_builder
        .set_usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        depth_image_ = depth_image_builder.build();

        for (auto image_view : image_views) {
            ImageView depth_image_view{ *logical_device_,
                                        depth_image_,
                                        swapchain_.get_extent(),
                                        VK_IMAGE_ASPECT_DEPTH_BIT };
            depth_image_views_.push_back(std::move(depth_image_view));
        }

        for (int i{ 0 }; i < image_views.size(); ++i) {
            // set of image views to make 1 framebuffer
            std::vector<ImageView*> views{};  // !!HERE? DANGLE?
            views.push_back(image_views[i]);
            views.push_back(&depth_image_views_[i]);

            Framebuffer framebuffer{ *logical_device_, *render_pass_, views };
            framebuffers_.push_back(std::move(framebuffer));
        }
    }
}  // namespace njin::vulkan
