#include "vulkan/SamplerBuilder.h"

namespace njin::vulkan {

    SamplerBuilder::SamplerBuilder(const LogicalDevice& device) :
        device_{ &device } {}

    Sampler SamplerBuilder::build() const {
        return { *device_, info_ };
    }

}  // namespace njin::vulkan