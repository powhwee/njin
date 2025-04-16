#include "vulkan/image_setup.h"

namespace njin::vulkan {
    std::vector<VkImageCreateInfo>
    make_image_create_set(const Swapchain& swapchain) {
        VkExtent2D extent_2d{ swapchain.get_extent() };
        VkImageCreateInfo depth{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = VK_FORMAT_D32_SFLOAT,
            .extent = { .width = extent_2d.width,
                        .height = extent_2d.height,
                        .depth = 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,  // ignored, sharing mode is exclusive
            .pQueueFamilyIndices =
            nullptr,  // ignored, sharing mode is exclusive
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };

        return { depth };
    }
}  // namespace njin::vulkan