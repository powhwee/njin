#include "vulkan/Renderer.h"

#include <memory>
#include <vector>

#include "vulkan/RenderResources.h"
#include "vulkan/SubpassModule.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    Renderer::Renderer(const LogicalDevice& logical_device,
                       const PhysicalDevice& physical_device,
                       Swapchain& swapchain,
                       const RendererInfo& info,
                       RenderResources& resources) :
        device_{ &logical_device },
        swapchain_{ &swapchain },
        image_available_semaphore_{ *device_ },
        render_finished_semaphore_{ *device_ },
        in_flight_fence_{ *device_ },
        resources_{ &resources } {
        // create the subpass modules for each subpass in each render pass
        for (const RenderPassInfo& render_pass_info : info.render_passes) {
            render_passes_.push_back(render_pass_info.name);
            for (const SubpassInfo& subpass_info : render_pass_info.subpasses) {
                // gather resources for the bind set
                VkPipeline pipeline{
                    resources.graphics_pipelines
                    .get_graphics_pipeline(render_pass_info.name,
                                           subpass_info.name)
                };
                VkPipelineLayout layout{
                    resources.graphics_pipelines
                    .get_pipeline_layout(render_pass_info.name,
                                         subpass_info.name)
                };

                std::vector<VkDescriptorSet> descriptor_sets{};
                for (const std::string& descriptor_set_name :
                     subpass_info.pipeline.set_layouts) {
                    descriptor_sets
                    .push_back(resources_->descriptor_sets
                               .get_descriptor_set(descriptor_set_name));
                }
                VkBuffer vertex_buffer{
                    resources.vertex_buffers
                    .get_vertex_buffer(subpass_info.vertex_buffer.name)
                };

                VkBuffer index_buffer = VK_NULL_HANDLE;
                if (subpass_info.index_buffer_name.has_value()) {
                    index_buffer = resources.index_buffers.get_index_buffer(subpass_info.index_buffer_name.value());
                }

                BindSet bind_set{ .pipeline = pipeline,
                                  .layout = layout,
                                  .descriptor_sets = descriptor_sets,
                                  .vertex_buffer = vertex_buffer,
                                  .index_buffer = index_buffer };

                // make the subpass module
                SubpassModule module(subpass_info.name, bind_set);
                subpass_modules_[render_pass_info.name]
                .push_back(std::move(module));
            }
        }
    }

    void Renderer::draw_frame(RenderInfos& render_infos) {
        vkWaitForFences(device_->get(),
                        1,
                        in_flight_fence_.get_handle_address(),
                        VK_TRUE,
                        UINT64_MAX);
        vkResetFences(device_->get(), 1, in_flight_fence_.get_handle_address());

        resources_->command_pool_.free_buffers();
        // get the index of the next image that will be available
        // NOTE: this does not mean the image can be written to
        // see https://docs.vulkan.org/spec/latest/chapters/VK_KHR_surface/wsi.html#vkAcquireNextImageKHR
        // "The presentation engine may not have finished reading from the image...
        // ...the application must use semaphore and/or fence..."
        uint32_t image_index{ 0 };
        vkAcquireNextImageKHR(device_->get(),
                              swapchain_->get(),
                              UINT64_MAX,
                              image_available_semaphore_.get(),
                              VK_NULL_HANDLE,
                              &image_index);
        CommandBuffer command_buffer{
            resources_->get_command_pool()
            .allocate_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        };

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        for (const std::string& render_pass_name : render_passes_) {
            run_render_pass(command_buffer,
                            render_pass_name,
                            image_index,
                            render_infos);
        }
        std::vector<VkSemaphore> wait_semaphores{
            image_available_semaphore_.get()
        };
        std::vector<VkPipelineStageFlags> wait_stages{
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };
        std::vector<VkSemaphore> signal_semaphores{
            render_finished_semaphore_.get()
        };
        VkFence signal_fence{ in_flight_fence_.get() };
        command_buffer.end();
        CommandBufferSubmitInfo submit_info{
            .wait_semaphores = wait_semaphores,
            .wait_stages = wait_stages,
            .signal_semaphores = signal_semaphores,
            .signal_fence = signal_fence,
            .should_wait_idle = false
        };
        command_buffer.submit(submit_info);
        // present when semaphores signalled (rendering done)
        VkPresentInfoKHR present_info{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .pNext = nullptr,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = render_finished_semaphore_.get_handle_address(),
            .swapchainCount = 1,
            .pSwapchains = swapchain_->get_handle_address(),
            .pImageIndices = &image_index,
            .pResults = nullptr
        };

        vkQueuePresentKHR(device_->get_present_queue(), &present_info);
    }

    Renderer::~Renderer() {
        vkDeviceWaitIdle(device_->get());
    }

    void Renderer::run_render_pass(CommandBuffer& command_buffer,
                                   const std::string& render_pass_name,
                                   uint32_t image_index,
                                   const RenderInfos& render_infos) const {
        // get the framebuffer to use
        const FramebufferSet* framebuffer_set{
            resources_->framebuffers.get_framebuffer_set(render_pass_name)
        };
        VkFramebuffer framebuffer{
            framebuffer_set->get_framebuffer(image_index)
        };

        VkClearValue color_clear_value{ .color = { 0.f, 0.f, 0.f, 1.f } };
        VkClearValue depth_clear_value{ .depthStencil = { 1.f, 0 } };
        std::vector<VkClearValue> clear_values{ color_clear_value,
                                                depth_clear_value };

        const RenderPass& render_pass{
            resources_->render_passes.get_render_pass(render_pass_name)
        };

        VkRenderPassBeginInfo begin_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = render_pass.get(),
            .framebuffer = framebuffer,
            .renderArea = { .offset = { .x = 0, .y = 0 },
                            .extent = framebuffer_set->get_extent() },
            .clearValueCount = 2,
            .pClearValues = clear_values.data()
        };

        vkCmdBeginRenderPass(command_buffer.get(),
                             &begin_info,
                             VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        std::vector<SubpassModule> subpass_modules{
            subpass_modules_.at(render_pass_name)
        };
        for (int i{ 0 }; i < subpass_modules.size(); ++i) {
            CommandBuffer subpass_command_buffer{
                resources_->get_command_pool()
                .allocate_buffer(VK_COMMAND_BUFFER_LEVEL_SECONDARY)
            };
            InheritanceInfo inheritance_info{
                .render_pass = render_pass.get(),
                .subpass = static_cast<uint32_t>(i),
                .framebuffer = framebuffer,
                .extent = framebuffer_set->get_extent(),
            };

            // retrieve the render infos to be processed by this subpass
            RenderKey key{ render_pass_name, subpass_modules[i].get_name() };
            std::vector<RenderInfo> render_info_list{
                render_infos.get_render_infos(key)
            };

            subpass_modules[i].record(subpass_command_buffer,
                                      inheritance_info,
                                      render_info_list);

            // retrieve the recorded commands and execute them
            vkCmdExecuteCommands(command_buffer.get(),
                                 1,
                                 subpass_modules[i].get_commands());
            if (i != subpass_modules.size() - 1) {
                // after the last subpass we do not need to call vkCmdNextSubpass
                vkCmdNextSubpass(command_buffer.get(),
                                 VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
            }
        }

        vkCmdEndRenderPass(command_buffer.get());
    }
}  // namespace njin::vulkan
