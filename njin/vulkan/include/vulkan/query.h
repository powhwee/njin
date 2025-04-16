#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

/**
 * Helper structs for interfacing with Vulkan API
 */
namespace njin::vulkan {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

    SwapChainSupportDetails query_swap_chain_support(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface);
}
