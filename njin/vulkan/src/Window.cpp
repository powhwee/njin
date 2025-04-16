#include <iostream>
#include <ostream>
#include <stdexcept>

#include "vulkan/window.h"


namespace njin::vulkan {
    Window::Window(const std::string& name,
                   int width,
                   int height,
                   SDL_WindowFlags flags) {
        window_ = SDL_CreateWindow(name.c_str(), width, height, flags);
        if (!window_) {
            std::cerr << "Failed to create window: " << SDL_GetError() <<
            std::endl;
        }
    }

    Window::Window(const Window& window) {
        window_ = window.window_;
    }

    Window::Window(Window&& window) {
        window_ = window.window_;
    }

    Window& Window::operator=(const Window& window) {
        window_ = window.window_;
        return *this;
    }

    Window& Window::operator=(Window&& window) {
        window_ = window.window_;
        return *this;
    }

    Window::~Window() {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }

    std::vector<const char*> Window::get_extensions() {
        std::vector<const char*> result{};
        uint32_t count{ 0 };
        const char* const* extensions{
            SDL_Vulkan_GetInstanceExtensions(&count)
        };

        for (uint32_t i{ 0 }; i < count; ++i) {
            result.emplace_back(extensions[i]);
        }

        return result;
    }

    SDL_Window* Window::get() const {
        return window_;
    }
}
