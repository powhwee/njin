#pragma once

#include <bitset>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "Image.h"
#include "LogicalDevice.h"

namespace njin::vulkan {
    /**
    * Builder for a VkImage
    * One-time use only. Use a new builder for each image
    */
    class ImageBuilder {
        public:
        ImageBuilder(const PhysicalDevice& physical_device,
                     const LogicalDevice& device);

        Image build();

        void set_image_type(VkImageType type);
        void set_format(VkFormat format);
        /**
         * Set the extent of the image
         * @param extent Extent of the image
         * @note Even if the format of the image is 2D, the depth member
         * is needed (1 in that case). In any case, the depth member needs
         * to be filled in
         */
        void set_extent(VkExtent3D extent);
        void set_initial_layout(VkImageLayout layout);
        void set_usage(VkImageUsageFlags usage);

        private:
        void set_queue_family(uint32_t queue_family_index);
        const LogicalDevice* device_{ nullptr };
        const PhysicalDevice* physical_device_{ nullptr };
        std::vector<uint32_t> queue_family_indices_{};
        VkImageCreateInfo info_{ .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .imageType = VK_IMAGE_TYPE_1D,
                                 .format = VK_FORMAT_UNDEFINED,
                                 .extent = {},
                                 .mipLevels = 1,
                                 .arrayLayers = 1,
                                 .samples = VK_SAMPLE_COUNT_1_BIT,
                                 .tiling = VK_IMAGE_TILING_OPTIMAL,
                                 .usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                                 .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 .queueFamilyIndexCount = 1,
                                 .pQueueFamilyIndices = nullptr,
                                 .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED };

        // image type
        // format
        // extent
        // queue family
        // initial layout
        // usage
        std::bitset<6> check_{};
    };

}  // namespace njin::vulkan
