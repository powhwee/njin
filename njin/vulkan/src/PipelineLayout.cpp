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

    PipelineLayout::PipelineLayout(const LogicalDevice& device,
                                   const PipelineInfo& pipeline_info,
                                   const DescriptorSets& descriptor_sets) :
        device_{ device.get() } {
        // pick out the descriptor sets
        std::vector<std::string> set_layout_names{ pipeline_info.set_layouts };
        std::vector<VkDescriptorSetLayout> set_layouts{};
        for (const std::string& set_layout_name : set_layout_names) {
            set_layouts.push_back(descriptor_sets
                                  .get_descriptor_set_layout(set_layout_name));
        }

        VkPipelineLayoutCreateInfo create_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data(),
            .pushConstantRangeCount =
            static_cast<uint32_t>(pipeline_info.push_constant_ranges.size()),
            .pPushConstantRanges = pipeline_info.push_constant_ranges.data()
        };

        if (vkCreatePipelineLayout(device_,
                                   &create_info,
                                   nullptr,
                                   &pipeline_layout_) != VK_SUCCESS) {
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
