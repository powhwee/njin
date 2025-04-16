#pragma once
#include "vulkan/GraphicsPipeline.h"
#include "vulkan/GraphicsPipelineBuilder.h"
#include "vulkan/PipelineLayout.h"
#include "vulkan/PipelineLayoutBuilder.h"

/**
 *  Helper methods for graphics pipeline setup to avoid cluttering main.cpp
 */

namespace njin::vulkan {
    GraphicsPipeline make_debug_pipeline(const LogicalDevice& logical_device,
                                         const RenderPass& render_pass,
                                         const PipelineLayout& pipeline_layout);

}  // namespace njin::vulkan
