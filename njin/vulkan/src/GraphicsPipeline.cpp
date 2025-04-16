#include "vulkan/GraphicsPipeline.h"

#include <stdexcept>
#include <string>

#include <vulkan/Renderer.h>
#include <vulkan/vulkan_core.h>

#include "vulkan/Buffer.h"
#include "vulkan/ShaderModule.h"
#include "vulkan/pipeline_util.h"

namespace {
    using namespace njin::vulkan;
}  // namespace

namespace njin::vulkan {

    GraphicsPipeline::GraphicsPipeline(const LogicalDevice& device,
                                       const VkGraphicsPipelineCreateInfo&
                                       info) :
        device_{ device.get() },
        layout_{ info.layout } {
        if (vkCreateGraphicsPipelines(device_,
                                      VK_NULL_HANDLE,
                                      1,
                                      &info,
                                      nullptr,
                                      &pipeline_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
    }

    GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept :
        device_{ other.device_ },
        pipeline_{ other.pipeline_ } {
        other.device_ = VK_NULL_HANDLE;
        other.pipeline_ = VK_NULL_HANDLE;
    }

    GraphicsPipeline::GraphicsPipeline(const LogicalDevice& device,
                                       VkPipeline pipeline) :
        device_{ device.get() },
        pipeline_{ pipeline } {}

    GraphicsPipeline::~GraphicsPipeline() {
        if (pipeline_ != VK_NULL_HANDLE) {
            vkDestroyPipeline(device_, pipeline_, nullptr);
        }
    }

    VkPipeline GraphicsPipeline::get() const {
        return pipeline_;
    }

    VkPipelineLayout GraphicsPipeline::get_layout() const {
        return layout_;
    }
}  // namespace njin::vulkan
