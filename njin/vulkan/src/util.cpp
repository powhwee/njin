#include "vulkan/util.h"
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <stdexcept>

#include <stb_image.h>

#include <algorithm>

namespace njin::vulkan {
    std::set<std::string>
    get_physical_device_extensions(const PhysicalDevice& physical_device) {
        uint32_t property_count{ 0 };
        vkEnumerateDeviceExtensionProperties(physical_device.get(),
                                             nullptr,
                                             &property_count,
                                             nullptr);

        std::vector<VkExtensionProperties> extensions(property_count);
        vkEnumerateDeviceExtensionProperties(physical_device.get(),
                                             nullptr,
                                             &property_count,
                                             extensions.data());

        std::set<std::string> extension_set{};
        for (const auto& extension : extensions) {
            extension_set.insert(extension.extensionName);
        }

        return extension_set;
    }

    bool check_physical_device_extension_support(
    const PhysicalDevice& physical_device,
    const std::vector<const char*>& requested_extensions) {
        std::set<std::string> available_extensions{
            get_physical_device_extensions(physical_device)
        };
        size_t count{ requested_extensions.size() };
        // count fulfilled extensions
        for (const auto& extension : requested_extensions) {
            if (available_extensions.contains(extension)) {
                --count;
            }
        }
        return count == 0;
    }

    VkSurfaceCapabilitiesKHR
    get_surface_capabilities(const PhysicalDevice& physical_device,
                             const Surface& surface) {
        VkSurfaceCapabilitiesKHR capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device.get(),
                                                  surface.get(),
                                                  &capabilities);
        return capabilities;
    }

    std::vector<VkSurfaceFormatKHR>
    get_surface_formats(const PhysicalDevice& physical_device,
                        const Surface& surface) {
        uint32_t surface_format_count{ 0 };
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.get(),
                                             surface.get(),
                                             &surface_format_count,
                                             nullptr);

        std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device.get(),
                                             surface.get(),
                                             &surface_format_count,
                                             surface_formats.data());

        return surface_formats;
    }

    std::vector<VkPresentModeKHR>
    get_surface_presentation_modes(const PhysicalDevice& physical_device,
                                   const Surface& surface) {
        uint32_t presentation_mode_count{ 0 };
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.get(),
                                                  surface.get(),
                                                  &presentation_mode_count,
                                                  nullptr);

        std::vector<VkPresentModeKHR> present_modes(presentation_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device.get(),
                                                  surface.get(),
                                                  &presentation_mode_count,
                                                  present_modes.data());

        return present_modes;
    }

    uint32_t find_memory_type(const PhysicalDevice& physical_device,
                              uint32_t type_filter,
                              VkMemoryPropertyFlags properties) {
        // find out what kind of memory our physical device has
        VkPhysicalDeviceMemoryProperties memory_properties{};
        vkGetPhysicalDeviceMemoryProperties(physical_device.get(),
                                            &memory_properties);

        for (uint32_t i{ 0 }; i < memory_properties.memoryTypeCount; ++i) {
            if (type_filter & (1 << i) &&
                (memory_properties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type");
    }

    VkSurfaceFormatKHR
    choose_surface_format(const std::vector<VkSurfaceFormatKHR>&
                          available_formats) {
        for (const auto& available_format : available_formats) {
            if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                available_format.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return available_format;
            }
        }
        // otherwise just use the first one available
        return available_formats[0];
    }
}  // namespace njin::vulkan
