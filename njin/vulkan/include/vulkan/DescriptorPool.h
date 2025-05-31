#pragma once
#include <vulkan/vulkan_core.h>

#include "DescriptorSetLayout.h"
#include "vulkan/DescriptorSet.h"
#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    class DescriptorPool {
        public:
        DescriptorPool() = default;
        /**
         * Constructor
         * @param device Logical device this pool associates with
         * @param physical_device Physical device this pool associates with
         * @param info Pool creation info
         */
        DescriptorPool(const LogicalDevice& device,
                       const PhysicalDevice& physical_device,
                       const VkDescriptorPoolCreateInfo& info);

        /**
         * Constructor. The constructed pool will be large enough to
         * allocate all given descriptor sets at once.
         * @param device Logical device
         * @param set_layout_infos SetLayoutInfos
         *
         */
        DescriptorPool(const LogicalDevice& device,
                       const std::vector<SetLayoutInfo>& set_layout_infos);
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;
        DescriptorPool(DescriptorPool&&) noexcept;
        DescriptorPool& operator=(DescriptorPool&&) noexcept;
        ~DescriptorPool();

        /**
       * Allocate a descriptor set from this pool
       * @return Allocated descriptor set
       */
        DescriptorSet
        allocate_descriptor_set(const DescriptorSetLayout& layout) const;

        VkDescriptorPool get() const;

        private:
        const LogicalDevice* logical_device_{ nullptr };
        const PhysicalDevice* physical_device_{ nullptr };
        VkDevice device_{ VK_NULL_HANDLE };
        VkDescriptorPool pool_{ VK_NULL_HANDLE };
    };

}  // namespace njin::vulkan