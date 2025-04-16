#pragma once

#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    /**
     * Wrapper around VkSemaphore
     */
    class Semaphore {
    public:
        Semaphore(const LogicalDevice& device);

        Semaphore(const Semaphore& other) = delete;

        Semaphore& operator=(const Semaphore& other) = delete;

        Semaphore(Semaphore&& other) = delete;

        Semaphore& operator=(Semaphore&& other) = delete;

        ~Semaphore();

        VkSemaphore get() const;

        VkSemaphore* get_handle_address();

    private:
        const VkDevice device_{ VK_NULL_HANDLE };
        VkSemaphore semaphore_{ VK_NULL_HANDLE };
    };
}
