#pragma once
#include <vector>

#include <vulkan/vulkan_core.h>

#include "PipelineLayout.h"
#include "vulkan/DescriptorSetLayout.h"

namespace njin::vulkan {
    class PipelineLayoutBuilder {
        public:
        /**
         * Constructor
         * @param device Logical device
         */
        PipelineLayoutBuilder(const LogicalDevice& device);

        /**
         * Add a descriptor set layout to the pipeline layout
         * @param layout Desciptor set layout
         */
        void add_descriptor_set_layout(const DescriptorSetLayout& layout);

        /**
         * Add a push constant range to the pipeline layout
         * @param range Push constant range
         */
        void add_push_constant_range(const VkPushConstantRange& range);

        /**
         * Build the pipeline layout object
         * @return Pipeline layout object
         */
        PipelineLayout build() const;

        private:
        const LogicalDevice& device_;

        std::vector<VkDescriptorSetLayout> set_layouts_{};
        std::vector<VkPushConstantRange> push_constant_ranges_{};
        VkPipelineLayoutCreateInfo info_{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = 0,
            .pSetLayouts = set_layouts_.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = push_constant_ranges_.data()
        };
    };
}  // namespace njin::vulkan