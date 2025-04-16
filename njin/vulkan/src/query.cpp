#include "vulkan/query.h"
#include <vector>

namespace njin::vulkan {
    SwapChainSupportDetails query_swap_chain_support(
        VkPhysicalDevice physical_device,
        VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        // Capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device,
                                                  surface,
                                                  &details.capabilities);

        // Surface formats
        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                             surface,
                                             &format_count,
                                             nullptr);

        if (format_count != 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device,
                                                 surface,
                                                 &format_count,
                                                 details.formats.data());
        }

        // Present modes
        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                                                  surface,
                                                  &present_mode_count,
                                                  nullptr);

        if (present_mode_count != 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device,
                surface,
                &present_mode_count,
                details.present_modes.data());
        }
        return details;
    }
}
