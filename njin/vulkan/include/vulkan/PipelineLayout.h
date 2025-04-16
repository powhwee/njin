#pragma once
#include <vulkan/vulkan_core.h>

#include "LogicalDevice.h"

namespace njin::vulkan {
    class PipelineLayout {
        public:
        PipelineLayout(const LogicalDevice& device,
                       VkPipelineLayoutCreateInfo create_info);

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
