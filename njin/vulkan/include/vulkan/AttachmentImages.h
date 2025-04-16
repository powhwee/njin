#pragma once
#include <unordered_map>

#include "vulkan/Image.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/Swapchain.h"
#include "vulkan/config.h"

namespace njin::vulkan {
    /**
     * Storage and management of VkImages used as attachments in framebuffers
     */
    class AttachmentImages {
        public:
        /**
         * Constructor
         * @param attachment_image_infos List of infos to use to make the
         * attachment images
         */
        explicit AttachmentImages(const LogicalDevice& device,
                                  const PhysicalDevice& physical_device,
                                  Swapchain& swapchain,
                                  const std::vector<AttachmentImageInfo>&
                                  infos);

        VkImageCreateInfo get_image_create_info(const std::string& name) const;

        /**
         * Create a set of image views of a particular image. Swapchains
         * create multiple presentation image views. Supposing the swapchain has
         * 4 image views, then 4 framebuffers are needed for the render pass.
         * This implies that each other attachment the render pass intends
         * to use should also be created from each framebuffer.
         *
         * This method creates all the needed image views by creating one set of
         * image views per swapchain image.
         * @param name Name of image to create an image view from
         * @return Set of image views, suitable for use in framebuffers
         */
        std::vector<std::vector<VkImageView>>
        make_image_view_sets(const std::vector<std::string>& names);

        VkExtent2D get_extent() const;

        private:
        const LogicalDevice* device_;
        Swapchain* swapchain_;
        // storage for image views; the underlying handles are used by other
        // classes but the owner is this one
        std::vector<ImageView> image_views_;
        std::unordered_map<std::string, VkImageCreateInfo> image_infos_;
        std::unordered_map<std::string, Image> images_;
    };
}  // namespace njin::vulkan