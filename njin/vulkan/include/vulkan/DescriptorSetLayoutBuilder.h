#pragma once

#include <unordered_map>
#include <variant>
#include <vector>

#include <math/njMat4.h>
#include <vulkan/vulkan_core.h>

#include "DescriptorSetLayout.h"

namespace njin::vulkan {

    /** Types used in set binding */
    struct Mvp {
        math::njMat4f model;
        math::njMat4f view;
        math::njMat4f projection;
    };

    /**
   * Builder for VkDescriptorSetLayout
   */
    class DescriptorSetLayoutBuilder {
        public:
        /**
         * Constructor
         * @param device Logical device
         */
        DescriptorSetLayoutBuilder(const LogicalDevice* device);

        /**
        * Add a single binding to the set layout.
        * @param name User-provided unique name to refer to this binding
        * within this descriptor set with
        * @param settings Descriptor set binding information
        */
        void add_binding(const std::string& name,
                         const DescriptorSetBindingSettings& settings);

        /**
         * Build the descriptor set layout
         * @return Descriptor set layout
         */
        DescriptorSetLayout build() const;

        /**
         * Reset this builder's state
         */
;       void reset();

        private:
        std::vector<VkDescriptorSetLayoutBinding> set_bindings_{};

        VkDescriptorSetLayoutCreateInfo info_{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .bindingCount = static_cast<uint32_t>(set_bindings_.size()),
            .pBindings = set_bindings_.data()
        };

        const LogicalDevice* device_{ nullptr };
        std::vector<VkDescriptorPoolSize> pool_sizes_{};

        std::unordered_map<std::string, DescriptorSetBindingSettings>
        binding_map_{};
    };

}  // namespace njin::vulkan