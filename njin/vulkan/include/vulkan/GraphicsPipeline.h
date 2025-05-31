#pragma once
#include "vulkan/LogicalDevice.h"
#include "vulkan/PipelineLayout.h"
#include "vulkan/RenderPass.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    class GraphicsPipeline {
        public:
        GraphicsPipeline(const LogicalDevice& device,
                         const VkGraphicsPipelineCreateInfo& info);

        /**
         * Constructor
         * @param device Logical device
         * @param render_pass Render pass this pipeline will be used with
         * @param pipeline_info Pipeline info
         */
        GraphicsPipeline(const LogicalDevice& device,
                         const RenderPass& render_pass,
                         const DescriptorSets& descriptor_sets,
                         const PipelineInfo& pipeline_info);

        GraphicsPipeline(const GraphicsPipeline& other) = delete;

        GraphicsPipeline& operator=(const GraphicsPipeline& other) = delete;

        GraphicsPipeline(GraphicsPipeline&& other) noexcept;

        GraphicsPipeline& operator=(GraphicsPipeline&& other) = delete;

        GraphicsPipeline(const LogicalDevice& device, VkPipeline pipeline);

        ~GraphicsPipeline();

        VkPipeline get() const;
        VkPipelineLayout get_layout() const;

        private:
        VkDevice device_{ VK_NULL_HANDLE };
        VkPipeline pipeline_{ VK_NULL_HANDLE };
        VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
