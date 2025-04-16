#include "vulkan/PhysicalDevice.h"

#include <optional>
#include <stdexcept>

#include <vulkan/Query.h>
#include <vulkan/util.h>

using namespace njin::vulkan;

namespace {
    /**
     * Retrieves all the physical devices on this system
     * @param instance Intialized vulkan instance
     * @return List of physical devices
     */
    std::vector<VkPhysicalDevice>
    get_physical_devices(const Instance& instance);

    /**
     * Retrieves the queue family properties for a given physical device
     * @param physical_device Physical device to query
     * @return List of queue family properties
     */
    std::vector<VkQueueFamilyProperties>
    get_queue_families(const VkPhysicalDevice physical_device);

    /**
     * Check if a physical device is suitable for our uses
     * Specifically, it must
     * 1. have a queue family that supports graphics operations
     * 2. that queue family must support presentation to our surface
     * @param device Physical device to check
     * @param surface Surface the device should support
     * @return Indices of the suitable queue families
     */
    QueueFamilyIndices find_family_indices(const VkPhysicalDevice device,
                                           const VkSurfaceKHR surface);

    std::vector<VkPhysicalDevice>
    get_physical_devices(const Instance& instance) {
        // Get the number of physical devices first
        uint32_t device_count{ 0 };
        vkEnumeratePhysicalDevices(*instance.get(), &device_count, nullptr);
        if (device_count == 0) {
            throw std::runtime_error(
            "Failed to find GPUs with Vulkan support.");
        }

        // Populate list of physical devices
        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(*instance.get(),
                                   &device_count,
                                   devices.data());

        return devices;
    }

    std::vector<VkQueueFamilyProperties>
    get_queue_families(const VkPhysicalDevice physical_device) {
        // Find the number of queue families first
        uint32_t queue_family_count{ 0 };
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                                 &queue_family_count,
                                                 nullptr);

        // Then get the queue families
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device,
                                                 &queue_family_count,
                                                 queue_families.data());

        return queue_families;
    }

    QueueFamilyIndices find_family_indices(const VkPhysicalDevice device,
                                           const VkSurfaceKHR surface) {
        auto queues = get_queue_families(device);
        QueueFamilyIndices indices{};
        // find a queue that supports graphics
        uint32_t current_index{ 0 };
        for (const auto& queue : queues) {
            // is a queue that supports graphics
            if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphics_family = current_index;
                break;
            }
            ++current_index;
        }

        // find a queue that supports present
        current_index = 0;
        for (const auto& queue : queues) {
            // check present support
            VkBool32 does_support{ false };
            vkGetPhysicalDeviceSurfaceSupportKHR(device,
                                                 current_index,
                                                 surface,
                                                 &does_support);

            if (does_support) {
                indices.present_family = current_index;
            }
            break;
        }

        return indices;
    }
}  // namespace

namespace njin::vulkan {
    bool QueueFamilyIndices::is_complete() const {
        return graphics_family.has_value() && present_family.has_value();
    }

    PhysicalDevice::PhysicalDevice(const Instance& instance,
                                   const Surface& surface) {
        std::vector<VkPhysicalDevice> physical_devices{
            get_physical_devices(instance)
        };

        for (const auto device : physical_devices) {
            QueueFamilyIndices indices{ find_family_indices(device,
                                                            surface.get()) };

            if (indices.is_complete()) {
                device_ = device;
                indices_ = indices;
            }
        }

        if (!device_) {
            throw std::runtime_error("Failed to create physical device");
        }
    }

    VkPhysicalDevice PhysicalDevice::get() const {
        return device_;
    }

    QueueFamilyIndices PhysicalDevice::get_indices() const {
        return indices_;
    }

    uint32_t PhysicalDevice::get_graphics_family_index() const {
        return indices_.graphics_family.value();
    }
}  // namespace njin::vulkan
