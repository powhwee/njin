#include "vulkan/SubpassModule.h"

namespace njin::vulkan {
    SubpassModule::SubpassModule(const std::string& name,
                                 const BindSet& bind_set) :
        name_{ name },
        bind_set_{ bind_set } {}

    void SubpassModule::record(CommandBuffer& command_buffer,
                               InheritanceInfo inheritance,
                               const std::vector<RenderInfo>& render_infos) {
        VkCommandBufferInheritanceInfo inheritance_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
            .pNext = nullptr,
            .renderPass = inheritance.render_pass,
            .subpass = inheritance.subpass,
            .framebuffer = inheritance.framebuffer,
            .occlusionQueryEnable = false,
            .queryFlags = 0,
            .pipelineStatistics = 0
        };
        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
                             inheritance_info);

        // configure the parts of the pipeline we declared dynamic
        VkViewport viewport{
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<float>(inheritance.extent.width),
            .height = static_cast<float>(inheritance.extent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f
        };

        vkCmdSetViewport(command_buffer.get(), 0, 1, &viewport);

        VkRect2D scissor{ .offset = { 0, 0 }, .extent = inheritance.extent };
        vkCmdSetScissor(command_buffer.get(), 0, 1, &scissor);

        vkCmdBindPipeline(command_buffer.get(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          bind_set_.pipeline);

        VkDeviceSize offsets{ 0 };
        vkCmdBindVertexBuffers(command_buffer.get(),
                               0,
                               1,
                               &(bind_set_.vertex_buffer),
                               &offsets);
        vkCmdBindIndexBuffer(command_buffer.get(),
                             bind_set_.index_buffer,
                             0,
                             VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(command_buffer.get(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                bind_set_.layout,
                                0,
                                static_cast<uint32_t>(bind_set_.descriptor_sets
                                                      .size()),
                                bind_set_.descriptor_sets.data(),
                                0,
                                nullptr);

        for (const RenderInfo& render_info : render_infos) {
            if (render_info.type == RenderType::Mesh) {
                auto info{ std::get<MeshRenderInfo>(render_info.info) };

                vkCmdPushConstants(command_buffer.get(),
                                   bind_set_.layout,
                                   VK_SHADER_STAGE_VERTEX_BIT,
                                   0,
                                   4,
                                   &info.model_index);
                vkCmdDrawIndexed(command_buffer.get(),
                                 info.index_count,
                                 1,
                                 info.first_index,
                                 info.vertex_offset,
                                 0);
            } else if (render_info.type == RenderType::Billboard) {
                auto info{ std::get<BillboardRenderInfo>(render_info.info) };
                vkCmdPushConstants(command_buffer.get(),
                                   bind_set_.layout,
                                   VK_SHADER_STAGE_VERTEX_BIT,
                                   0,
                                   4,
                                   &info.model_index);
                vkCmdPushConstants(command_buffer.get(),
                                   bind_set_.layout,
                                   VK_SHADER_STAGE_FRAGMENT_BIT,
                                   4,
                                   4,
                                   &info.texture_index);
                vkCmdDraw(command_buffer.get(), 6, 1, info.billboard_offset, 0);
            }
        }

        CommandBufferSubmitInfo end_info{};
        command_buffer.end();
        commands_ = command_buffer.get();
    }

    std::string SubpassModule::get_name() const {
        return name_;
    }

    const VkCommandBuffer* SubpassModule::get_commands() const {
        return &commands_;
    }
}  // namespace njin::vulkan