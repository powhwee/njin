#pragma once
#include <string>

#include <vulkan/vulkan_core.h>

#include "CommandBuffer.h"
#include "DescriptorSet.h"
#include "RenderInfos.h"

namespace njin::vulkan {
    /**
     * List of things to bind within a subpass
     */
    struct BindSet {
        VkPipeline pipeline;
        VkPipelineLayout layout;
        std::vector<VkDescriptorSet> descriptor_sets;
        VkBuffer vertex_buffer;
        VkBuffer index_buffer;
    };

    /**
     * List of things the subpass needs to operate its own command buffer,
     * which was passed down from the parent render pass
     */
    struct InheritanceInfo {
        VkRenderPass render_pass;
        uint32_t subpass;
        VkFramebuffer framebuffer;
        VkExtent2D extent;
    };

    /**
     * Container for all the components needed to perform rendering
     * within one subpass
     *     - framebuffer
     *     - render pass / subpasses
     *     - graphics pipelines
     *     - descriptor sets
     *     - vertex buffers
     */
    class SubpassModule {
        public:
        /**
         * Constructor
         * @param command_buffer Secondary command buffer to record draw / bind commands with
         * @param bind_set Set of resources to bind during this subpass
         */
        SubpassModule(const std::string& name, const BindSet& bind_set);

        /**
         * Perform this subpass with the provided render infos
         * @param command_buffer Command buffer to record the draw/bind commands with
         * @param inheritance
         * @param render_infos
         * @note The provided command buffer must be in the initial state and a
         * secondary command buffer
         * // TODO render info param
         */
        void record(CommandBuffer& command_buffer,
                    InheritanceInfo inheritance,
                    const std::vector<RenderInfo>& render_infos);

        std::string get_name() const;

        const VkCommandBuffer* get_commands() const;

        private:
        BindSet bind_set_;
        VkCommandBuffer commands_{ VK_NULL_HANDLE };
        std::string name_;
    };
}  // namespace njin::vulkan