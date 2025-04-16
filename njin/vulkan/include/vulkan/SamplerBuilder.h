#pragma once

#include <vulkan/vulkan_core.h>

#include "vulkan/LogicalDevice.h"
#include "vulkan/Sampler.h"

namespace njin::vulkan {
    class SamplerBuilder {
        public:
        SamplerBuilder(const LogicalDevice& device);

        Sampler build() const;

        private:
        const LogicalDevice* device_;

        VkSamplerCreateInfo info_{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.f,
            .anisotropyEnable = VK_FALSE,  // TODO
            .maxAnisotropy = 1.f,
            .compareEnable = VK_FALSE,
            .compareOp = VK_COMPARE_OP_ALWAYS,
            .minLod = 0.f,
            .maxLod = 0.f,
            .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE
        };
    };
}  // namespace njin::vulkan