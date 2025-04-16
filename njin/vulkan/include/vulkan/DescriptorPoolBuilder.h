#pragma once
#include <memory>

#include <vulkan/vulkan_core.h>

#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "LogicalDevice.h"

namespace njin::vulkan {
    class DescriptorPoolBuilder {
        public:
        DescriptorPoolBuilder(const LogicalDevice& device,
                              const PhysicalDevice& physical_device);

        /**
         * Add an additional descriptor set that the built pool needs to support
         * @param descriptor_set_layout Descriptor set layout
         */
        void add_descriptor_set_layout(const DescriptorSetLayout&
                                       descriptor_set_layout);

        DescriptorPool build() const;

        private:
        const LogicalDevice* device_{ nullptr };
        const PhysicalDevice* physical_device_{ nullptr };

        /**
         * Add a single pool size
         * @param pool_size Pool size definition
         */
        void add_descriptor_pool_size(const VkDescriptorPoolSize& pool_size);

        std::vector<VkDescriptorPoolSize> pool_sizes_{};
        VkDescriptorPoolCreateInfo info_{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .maxSets = 0,
            .poolSizeCount = 0,
            .pPoolSizes = pool_sizes_.data()
        };
    };

}  // namespace njin::vulkan