#pragma once
#include "LogicalDevice.h"
#include "vulkan/Buffer.h"
#include "vulkan/CommandBuffer.h"

namespace njin::vulkan {
    using ImageWidth = uint32_t;
    using ImageHeight = uint32_t;

    /**
   * Wrapper over VkImage
   */
    class Image {
        public:
        /**
         * Constructor
         * @param device Logical device
         * @param info Image create info used to create the VkImage
         */
        Image(const LogicalDevice& device,
              const PhysicalDevice& physical_device,
              const VkImageCreateInfo& info);
        Image() = default;

        /**
         * Constructor from a live VkImage handle. Most commonly used for
         * VkImage handles provided directly by vkGetSwapchainImagesKHR
         */
        Image(const LogicalDevice& device, VkImage image, VkFormat format);
        Image(const Image& other) = delete;
        Image operator=(const Image& other) = delete;
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;
        ~Image();

        VkImage get() const;

        VkFormat get_format() const;

        ImageWidth get_width() const;
        ImageHeight get_height() const;

        /**
       * Transition the layout of this image from its current layout
       * to a specified layout immediately
       * @param new_layout New layout to transition to
       */
        void transition_layout(VkImageLayout new_layout);

        /**
       * Load the contents of the buffer into this image's memory
       * @param buffer Buffer contents to load
       */
        void load_buffer(const Buffer& buffer);

        private:
        const LogicalDevice* device_;
        const PhysicalDevice* physical_device_;
        VkFormat format_{ VK_FORMAT_UNDEFINED };
        VkImage image_{ VK_NULL_HANDLE };
        VkDeviceMemory memory_{};
        VkImageCreateInfo info_{};
        VkImageLayout current_layout_{ VK_IMAGE_LAYOUT_UNDEFINED };

        // do not destroy images that were created by the swapchain,
        // in this special case the swapchain will do it for us
        bool destroy_{ true };
    };
}  // namespace njin::vulkan
