#pragma once
#include "Swapchain.h"
#include "vulkan/ImageBuilder.h"

/**
 * Utilities for specifying the list of VkImages (besides the swapchain image)
 * that will be renderered to/from.
 * This means the set of images that need to be created in each framebuffer,
 * and referenced in each render pass.
*/
namespace njin::vulkan {
    /**
    *  **Ordered** set of VkImageCreateInfo that will be used
    * throughout the lifetime of the application. Images will be created
    * based on this provided set, and the ordering will be what will be used
    * in each framebuffer
    * @param swapchain Swapchain the images will be used with
    */
    std::vector<VkImageCreateInfo>
    make_image_create_set(const Swapchain& swapchain);

};  // namespace njin::vulkan