#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan/DescriptorSets.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/config_classes.h"

// TODO: remove, move functionality to GraphicsPipeline
namespace njin::vulkan {
    class PipelineLayout {
        public:
        PipelineLayout(const LogicalDevice& device,
                       VkPipelineLayoutCreateInfo create_info);

        PipelineLayout(const LogicalDevice& device,
                       const PipelineInfo& pipeline_info,
                       const DescriptorSets& descriptor_sets);

        PipelineLayout(const PipelineLayout& other) = delete;
        PipelineLayout& operator=(const PipelineLayout& other) = delete;
        PipelineLayout(PipelineLayout&& other) noexcept;
        PipelineLayout& operator=(PipelineLayout&& other) = delete;

        ~PipelineLayout();

        VkPipelineLayout get() const;

        private:
        VkDevice device_;
        VkPipelineLayout pipeline_layout_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan
