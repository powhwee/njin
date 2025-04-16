#pragma once
#include "vulkan/PhysicalDevice.h"

namespace njin::vulkan {
    class LogicalDevice {
    public:
        LogicalDevice(const PhysicalDevice& physical_device);

        LogicalDevice(const LogicalDevice& device) = delete;

        LogicalDevice& operator=(const LogicalDevice& device) = delete;

        LogicalDevice(LogicalDevice&& device) = delete;

        LogicalDevice& operator=(LogicalDevice&& device) = delete;

        ~LogicalDevice();

        const VkDevice get() const;

        const VkQueue get_graphics_queue() const;

        const VkQueue get_present_queue() const;

    private:
        // logical device
        VkDevice device_;

        VkQueue graphics_queue_{ VK_NULL_HANDLE };
        VkQueue present_queue_{ VK_NULL_HANDLE };
    };
}
