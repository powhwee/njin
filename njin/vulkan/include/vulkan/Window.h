#pragma once

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <string>
#include <vulkan/vulkan_core.h>
#include <vector>

namespace njin::vulkan {
    class Window {
    public:
        // Constructors
        Window(const std::string& name,
               int width,
               int height,
               SDL_WindowFlags flags);

        Window(const Window& window);

        Window(Window&& window);

        Window& operator=(const Window& window);

        Window& operator=(Window&& window);

        ~Window();

        /**
         * Gets the Vulkan instance extensions required to use this window
         * @return R
         */
        std::vector<const char*> get_extensions();

        SDL_Window* get() const;

    private:
        SDL_Window* window_ = nullptr;
    };
}
