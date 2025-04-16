#pragma once
#include <vulkan/vulkan_core.h>

#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    class Sampler {
        public:
        Sampler() = default;
        Sampler(const LogicalDevice& device, const VkSamplerCreateInfo& info);

        Sampler(const Sampler&) = delete;
        Sampler& operator=(const Sampler&) = delete;
        Sampler(Sampler&& other) noexcept;
        Sampler& operator=(Sampler&& other) noexcept;
        ~Sampler();

        VkSampler get() const;

        private:
        const LogicalDevice* device_{ nullptr };
        VkSampler sampler_{ VK_NULL_HANDLE };
    };
}  // namespace njin::vulkan