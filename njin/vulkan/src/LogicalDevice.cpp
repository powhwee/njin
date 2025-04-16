#include "vulkan/LogicalDevice.h"

#include <iostream>
#include <stdexcept>

#include <vulkan/util.h>

using namespace njin::vulkan;

// required physical device extensions
const std::vector<const char*> physical_device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
};

namespace {
    VkDeviceCreateInfo
    make_device_create_info(const PhysicalDevice& physical_device,
                            const std::vector<VkDeviceQueueCreateInfo>&
                            queue_create_infos,
                            const std::vector<const char*>& extension_names);

    /**
     * Make a VkDeviceQueueCreateInfo
     * @param priority Priority of queue
     * @param queue_family_index Queue family index to create this structure for
     * @return Queue create info
     */
    VkDeviceQueueCreateInfo make_queue_create_info(uint32_t queue_family_index,
                                                   const float* priority);

    /**
     * Make a list of queue create infos, one for each queue family we will use
     * @param indices Queue family indices
     * @return List of queue create infos
     */
    std::vector<VkDeviceQueueCreateInfo>
    make_queue_create_infos(const QueueFamilyIndices& indices);

    VkDeviceCreateInfo
    make_device_create_info(const PhysicalDevice& physical_device,
                            const std::vector<VkDeviceQueueCreateInfo>&
                            queue_create_infos,
                            const std::vector<const char*>& extension_names) {
        VkDeviceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        // Queue create
        info.queueCreateInfoCount = queue_create_infos.size();
        info.pQueueCreateInfos = queue_create_infos.data();

        // Extensions
        info.enabledExtensionCount = extension_names.size();
        info.ppEnabledExtensionNames = extension_names.data();

        // enable features we want
        VkPhysicalDeviceFeatures features{};
        features.samplerAnisotropy = VK_TRUE;
        features.fillModeNonSolid = VK_TRUE;

        return info;
    }

    VkDeviceQueueCreateInfo make_queue_create_info(uint32_t queue_family_index,
                                                   const float* priority) {
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.queueFamilyIndex = queue_family_index;
        info.queueCount = 1;  // we probably only need 1

        // we only have one queue so this priority doesn't really matter
        info.pQueuePriorities = priority;

        return info;
    }

    std::vector<VkDeviceQueueCreateInfo>
    make_queue_create_infos(const QueueFamilyIndices& indices) {
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

        constexpr float priority{ 1.0f };
        VkDeviceQueueCreateInfo graphics_queue_info{
            make_queue_create_info(indices.graphics_family.value(), &priority)
        };

        queue_create_infos.emplace_back(graphics_queue_info);

        // if the present and graphics family are in different families then
        // we need an additional queue for the present alone
        if (indices.graphics_family != indices.present_family) {
            VkDeviceQueueCreateInfo present_queue_info{
                make_queue_create_info(indices.present_family.value(),
                                       &priority)
            };
            queue_create_infos.emplace_back(present_queue_info);
        }

        return queue_create_infos;
    }
}  // namespace

namespace njin::vulkan {
    LogicalDevice::LogicalDevice(const PhysicalDevice& physical_device) {
        if (
        !check_physical_device_extension_support(physical_device,
                                                 physical_device_extensions)) {
            throw std::runtime_error(
            "Requested device extensions not supported by physical device");
        }

        QueueFamilyIndices indices{ physical_device.get_indices() };
        auto queue_create_infos{ make_queue_create_infos(indices) };

        // enable features we want
        VkPhysicalDeviceFeatures features{
            .fillModeNonSolid = VK_TRUE,
            .samplerAnisotropy = VK_TRUE,
        };

        VkDeviceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount =
            static_cast<uint32_t>(queue_create_infos.size()),
            .pQueueCreateInfos = queue_create_infos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount =
            static_cast<uint32_t>(physical_device_extensions.size()),
            .ppEnabledExtensionNames = physical_device_extensions.data(),
            .pEnabledFeatures = &features,
        };

        if (vkCreateDevice(physical_device.get(), &info, nullptr, &device_)) {
            throw std::runtime_error("Failed to create logical device");
        }

        // Retrieve the created queues
        vkGetDeviceQueue(device_,
                         indices.graphics_family.value(),
                         0,
                         &graphics_queue_);
        vkGetDeviceQueue(device_,
                         indices.present_family.value(),
                         0,
                         &present_queue_);
    }

    LogicalDevice::~LogicalDevice() {
        vkDestroyDevice(device_, nullptr);
    }

    const VkDevice LogicalDevice::get() const {
        return device_;
    }

    const VkQueue LogicalDevice::get_graphics_queue() const {
        return graphics_queue_;
    }

    const VkQueue LogicalDevice::get_present_queue() const {
        return present_queue_;
    }
}  // namespace njin::vulkan
