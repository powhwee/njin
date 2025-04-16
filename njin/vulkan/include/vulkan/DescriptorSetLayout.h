#pragma once
#include <unordered_map>
#include <variant>

#include <vulkan/vulkan_core.h>

#include "LogicalDevice.h"
#include "vulkan/config.h"

namespace njin::vulkan {
    /**
     * Specific data needed when creating a buffer binding
   */
    struct DescriptorSetBindingBufferSettings {
        // size of a single descriptor. Used to construct VkBufferInfo,
        // as each descriptor in a descriptor array needs to know the memory
        // range in the buffer it refers to
        size_t descriptor_size{ 0 };
    };

    /**
     * Specific data needed when creating an image binding
   */
    struct DescriptorSetBindingImageSetting {
        VkSampler sampler{ VK_NULL_HANDLE };
        std::vector<VkImageView> image_views{ VK_NULL_HANDLE };
        VkImageLayout image_layout{ VK_IMAGE_LAYOUT_UNDEFINED };
    };

    /**
   * Data for creating a VkDescriptorSetLayoutBinding
   * All members in this struct are common regardless of the descriptor type
   * used in the binding, *except* the extra settings member.
   * The extra settings member specifies descriptor-type-specific data
   */
    struct DescriptorSetBindingSettings {
        uint32_t binding{ 0 };  // binding number, referenced in shaders
        VkDescriptorType descriptor_type{ VK_DESCRIPTOR_TYPE_SAMPLER };
        uint32_t descriptor_count{ 0 };
        VkShaderStageFlags stage_flags{ VK_SHADER_STAGE_VERTEX_BIT };

        std::variant<DescriptorSetBindingBufferSettings,
                     DescriptorSetBindingImageSetting>
        extra_settings;
    };

    class DescriptorSetLayout {
        public:
        DescriptorSetLayout() = default;
        /**
         * Constructor
         * @param device Logical device
         * @param info VkDescriptorSetLayoutCreateInfo
         */
        DescriptorSetLayout(
        const LogicalDevice& device,
        const VkDescriptorSetLayoutCreateInfo& info,
        const std::unordered_map<std::string, DescriptorSetBindingSettings>&
        bindings);

        /**
         * Constructor
         * @param device Logical device
         * @param info VkDescriptorSetLayoutCreateInfo
         */
        DescriptorSetLayout(const LogicalDevice& device,
                            const VkDescriptorSetLayoutCreateInfo& info);

        DescriptorSetLayout(const LogicalDevice& device,
                            const SetLayoutInfo& info);
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
        DescriptorSetLayout& operator=(DescriptorSetLayout&& other) noexcept;
        ~DescriptorSetLayout();

        /**
         * Retrieve the underlying VkDescriptorSetLayout handle
         * @return VkDescriptorSetLayout handle
         */
        VkDescriptorSetLayout get() const;

        /**
       * Retrieve the handle to the VkDescriptorSetLayout handle
       * @return Handle to VkDescriptorSetLayout handle
        */
        const VkDescriptorSetLayout* get_handle() const;

        /**
       * Retrieve the list of pool sizes needed to support this set
       * @return List of required pool sizes
       */
        std::vector<VkDescriptorPoolSize> get_pool_sizes() const;

        /**
* Retrieve the name to layout binding map
* @return Name to layout binding map
*/
        std::unordered_map<std::string, DescriptorSetBindingSettings>
        get_binding_map() const;

        private:
        VkDescriptorSetLayout layout_{ VK_NULL_HANDLE };
        VkDevice device_{ VK_NULL_HANDLE };
        std::vector<VkDescriptorPoolSize> pool_sizes_{};
        std::unordered_map<std::string, DescriptorSetBindingSettings>
        binding_map_{};

        /**
       * Find out the list of pool sizes needed to support allocation of this
       * descriptor set
       * This is used for building a correctly configured VkDescriptorPool later,
       * lest the created pool can't actually support what our set(s) need
       * @param info Layout information for this descriptor set
       */
        void discover_pool_sizes(const VkDescriptorSetLayoutCreateInfo& info);
    };
}  // namespace njin::vulkan