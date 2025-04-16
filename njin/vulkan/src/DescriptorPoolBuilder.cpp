#include "vulkan/DescriptorPoolBuilder.h"

#include <memory>

namespace njin::vulkan {
    DescriptorPoolBuilder::DescriptorPoolBuilder(const LogicalDevice& device,
                                                 const PhysicalDevice&
                                                 physical_device) :
        device_{ &device },
        physical_device_{ &physical_device } {}

    void
    DescriptorPoolBuilder::add_descriptor_set_layout(const DescriptorSetLayout&
                                                     descriptor_set_layout) {
        std::vector<VkDescriptorPoolSize> pool_sizes{
            descriptor_set_layout.get_pool_sizes()
        };
        for (const auto& pool_size : pool_sizes) {
            add_descriptor_pool_size(pool_size);
        }
        info_.maxSets += 1;
    }

    void
    DescriptorPoolBuilder::add_descriptor_pool_size(const VkDescriptorPoolSize&
                                                    pool_size) {
        pool_sizes_.push_back(pool_size);
        info_.poolSizeCount = pool_sizes_.size();
        info_.pPoolSizes = pool_sizes_.data();
    }

    DescriptorPool DescriptorPoolBuilder::build() const {
        return { *device_, *physical_device_, info_ };
    }
}  // namespace njin::vulkan