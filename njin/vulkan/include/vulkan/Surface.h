#pragma once

#include "vulkan/Window.h"
#include "vulkan/Instance.h"

/**
 * Wrapper around VkSurfaceKHR
 */
namespace njin::vulkan {
    class Surface {
    public:
        Surface(const Instance& instance, const Window& window);

        Surface(const Surface& surface) = delete;

        Surface& operator=(const Surface& surface) = delete;

        Surface(Surface&& surface) = delete;

        Surface& operator=(Surface&& surface) = delete;

        ~Surface();

        VkSurfaceKHR get() const;

    private:
        VkSurfaceKHR surface_;
        const VkInstance* const instance_;
    };
}
