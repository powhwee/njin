#include "vulkan/PipelineLayoutBuilder.h"

namespace njin::vulkan {

    PipelineLayoutBuilder::PipelineLayoutBuilder(const LogicalDevice& device) :
        device_{ device } {}

    void
    PipelineLayoutBuilder::add_descriptor_set_layout(const DescriptorSetLayout&
                                                     layout) {
        VkDescriptorSetLayout handle{ layout.get() };
        set_layouts_.push_back(handle);
        info_.setLayoutCount = set_layouts_.size();
        info_.pSetLayouts = set_layouts_.data();
    }

    void
    PipelineLayoutBuilder::add_push_constant_range(const VkPushConstantRange&
                                                   range) {
        push_constant_ranges_.push_back(range);
        info_.pushConstantRangeCount = push_constant_ranges_.size();
        info_.pPushConstantRanges = push_constant_ranges_.data();
    }

    PipelineLayout PipelineLayoutBuilder::build() const {
        return { device_, info_ };
    }
}  // namespace njin::vulkan