#pragma once
#include <unordered_map>

#include <vulkan/vulkan_core.h>

#include "Buffer.h"
#include "DescriptorSetLayout.h"

#include <any>

namespace njin::vulkan {
    /**
     *  Wrapper around VkDescriptorSet
     *  Provides a clean interface for writing data to buffers bound to
     *  each of the descriptor set's bindings
     */
    class DescriptorSet {
        public:
        /**
         * Constructor
         * @param logical_device Logical device associated with this descriptor set
         * @param physical_device Physical device associated with this descriptor set
         * @param descriptor_set Allocated descriptor set
         * @param layout Descriptor set layout used to create the descriptor set
         * @note the layout is used to provide information on how to create the
         * buffer to bind to the descriptor set
         */
        DescriptorSet(const LogicalDevice& logical_device,
                      const PhysicalDevice& physical_device,
                      VkDescriptorSet descriptor_set,
                      const DescriptorSetLayout& layout);

        DescriptorSet() = default;
        VkDescriptorSet get() const;

        DescriptorSet(const DescriptorSet&) = delete;
        DescriptorSet& operator=(const DescriptorSet&) = delete;
        DescriptorSet(DescriptorSet&& other) noexcept;
        DescriptorSet& operator=(DescriptorSet&& other) noexcept;

        /**
       * Update the buffer for a particular descriptor with new data
       * @param name Name of binding whose buffer we want update
       * @param new_data New data to place into buffer
       * @note this replaces all existing data in the buffer
       */
        template<typename T>
        void update_descriptor_data(const std::string& name,
                                    const std::vector<T>& new_data);

        private:
        std::unordered_map<std::string, Buffer> buffers_{};
        VkDescriptorSet descriptor_set_{ VK_NULL_HANDLE };
    };

};  // namespace njin::vulkan