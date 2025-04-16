#include "vulkan/Sampler.h"

#include <stdexcept>

namespace njin::vulkan {
    Sampler::Sampler(const LogicalDevice& device,
                     const VkSamplerCreateInfo& info) :
        device_{ &device } {
        if (vkCreateSampler(device_->get(), &info, nullptr, &sampler_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create sampler");
        }
    }

    Sampler::Sampler(Sampler&& other) noexcept :
        device_{ other.device_ },
        sampler_{ other.sampler_ } {
        other = Sampler{};
    }

    Sampler& Sampler::operator=(Sampler&& other) noexcept {
        device_ = other.device_;
        sampler_ = other.sampler_;
        other.device_ = nullptr;
        other.sampler_ = VK_NULL_HANDLE;
        return *this;
    }

    Sampler::~Sampler() {
        if (device_ != nullptr) {
            vkDestroySampler(device_->get(), sampler_, nullptr);
        }
    }

    VkSampler Sampler::get() const {
        return sampler_;
    };

}  // namespace njin::vulkan