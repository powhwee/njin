#include "vulkan/Semaphore.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace njin::vulkan {
    Semaphore::Semaphore(const LogicalDevice& device) : device_{
        device.get()
    } {
        VkSemaphoreCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0
        };

        if (vkCreateSemaphore(device_, &info, nullptr, &semaphore_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphore!");
        }
    }

    Semaphore::~Semaphore() {
        vkDestroySemaphore(device_, semaphore_, nullptr);
    }

    VkSemaphore* Semaphore::get_handle_address() {
        return &semaphore_;
    }

    VkSemaphore Semaphore::get() const {
        return semaphore_;
    }
}
