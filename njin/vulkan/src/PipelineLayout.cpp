#include "vulkan/PipelineLayout.h"

#include <stdexcept>

namespace njin::vulkan {

    PipelineLayout::PipelineLayout(const LogicalDevice& device,
                                   VkPipelineLayoutCreateInfo create_info) :
        device_{ device.get() } {
        if (vkCreatePipelineLayout(device.get(),
                                   &create_info,
                                   nullptr,
                                   &pipeline_layout_)) {
            throw std::runtime_error("failed to create pipeline layout");
        }
    }

    PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept :
        device_{ other.device_ },
        pipeline_layout_{ other.pipeline_layout_ } {
        other.device_ = VK_NULL_HANDLE;
        other.pipeline_layout_ = VK_NULL_HANDLE;
    }

    PipelineLayout::~PipelineLayout() {
        if (pipeline_layout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
        }
    }

    VkPipelineLayout PipelineLayout::get() const {
        return pipeline_layout_;
    }
}  // namespace njin::vulkan
