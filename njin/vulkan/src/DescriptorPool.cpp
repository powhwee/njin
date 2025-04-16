#include "vulkan/DescriptorPool.h"

#include <stdexcept>

namespace njin::vulkan {

    DescriptorPool::DescriptorPool(const LogicalDevice& device,
                                   const PhysicalDevice& physical_device,
                                   const VkDescriptorPoolCreateInfo& info) :
        device_{ device.get() },
        logical_device_{ &device },
        physical_device_{ &physical_device } {
        if (vkCreateDescriptorPool(device_, &info, nullptr, &pool_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool");
        };
    }

    DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept :
        device_{ other.device_ },
        pool_{ other.pool_ } {
        other.device_ = VK_NULL_HANDLE;
        other.pool_ = VK_NULL_HANDLE;
    }

    DescriptorPool& DescriptorPool::operator=(DescriptorPool&& other) noexcept {
        device_ = other.device_;
        pool_ = other.pool_;
        other.device_ = VK_NULL_HANDLE;
        other.pool_ = VK_NULL_HANDLE;
        return *this;
    }

    DescriptorPool::~DescriptorPool() {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device_, pool_, nullptr);
        }
    }

    DescriptorSet
    DescriptorPool::allocate_descriptor_set(const DescriptorSetLayout& layout)
    const {
        VkDescriptorSetAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = nullptr,
            .descriptorPool = pool_,
            .descriptorSetCount = 1,
            .pSetLayouts = layout.get_handle()
        };
        VkDescriptorSet descriptor_set{ VK_NULL_HANDLE };
        if (vkAllocateDescriptorSets(device_, &info, &descriptor_set) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor set");
        }
        return { *logical_device_, *physical_device_, descriptor_set, layout };
    }

    VkDescriptorPool DescriptorPool::get() const {
        return pool_;
    }
}  // namespace njin::vulkan