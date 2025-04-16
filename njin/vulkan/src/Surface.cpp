#include "vulkan/Surface.h"

#include <stdexcept>

namespace njin::vulkan {
    Surface::Surface(const Instance& instance,
                     const Window& window) : instance_{ instance.get() } {
        const bool result{
            SDL_Vulkan_CreateSurface(window.get(),
                                     *instance_,
                                     nullptr,
                                     &surface_)
        };

        if (!result) {
            throw std::runtime_error("Failed to create surface!");
        }
    }

    Surface::~Surface() {
        SDL_Vulkan_DestroySurface(*instance_, surface_, nullptr);
        surface_ = VK_NULL_HANDLE;
    }

    VkSurfaceKHR Surface::get() const {
        return surface_;
    }
}
