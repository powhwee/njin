#include "vulkan/DescriptorSetLayout.h"

#include <stdexcept>
#include <unordered_map>

namespace njin::vulkan {

    DescriptorSetLayout::DescriptorSetLayout(
    const LogicalDevice& device,
    const VkDescriptorSetLayoutCreateInfo& info,
    const std::unordered_map<std::string, DescriptorSetBindingSettings>&
    binding_map) :
        device_{ device.get() },
        binding_map_{ binding_map } {
        if (vkCreateDescriptorSetLayout(device_, &info, nullptr, &layout_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
        discover_pool_sizes(info);
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&&
                                             other) noexcept :
        layout_{ other.layout_ },
        device_{ other.device_ } {
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
    }

    DescriptorSetLayout&
    DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept {
        layout_ = other.layout_;
        device_ = other.device_;
        other.layout_ = VK_NULL_HANDLE;
        other.device_ = VK_NULL_HANDLE;
        return *this;
    }

    DescriptorSetLayout::~DescriptorSetLayout() {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(device_, layout_, nullptr);
        }
    }

    VkDescriptorSetLayout DescriptorSetLayout::get() const {
        return layout_;
    }

    const VkDescriptorSetLayout* DescriptorSetLayout::get_handle() const {
        return &layout_;
    }

    std::vector<VkDescriptorPoolSize>
    DescriptorSetLayout::get_pool_sizes() const {
        return pool_sizes_;
    }

    std::unordered_map<std::string, DescriptorSetBindingSettings>
    DescriptorSetLayout::get_binding_map() const {
        return binding_map_;
    }

    void DescriptorSetLayout::discover_pool_sizes(
    const VkDescriptorSetLayoutCreateInfo& info) {
        for (uint32_t i{ 0 }; i < info.bindingCount; ++i) {
            // each binding will have its own associated pool size requirements
            const VkDescriptorSetLayoutBinding& binding{ info.pBindings[i] };
            VkDescriptorPoolSize pool_size{
                .type = binding.descriptorType,
                .descriptorCount = binding.descriptorCount
            };
            pool_sizes_.push_back(pool_size);
        }
    }

}  // namespace njin::vulkan