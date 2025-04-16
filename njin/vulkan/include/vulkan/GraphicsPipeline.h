#pragma once
#include "vulkan/LogicalDevice.h"
#include "vulkan/PipelineLayout.h"
#include "vulkan/RenderPass.h"

namespace njin::vulkan {
    class GraphicsPipeline {
        public:
        GraphicsPipeline(const LogicalDevice& device,
                         const VkGraphicsPipelineCreateInfo& info);

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
        VkPipelineLayout layout_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
