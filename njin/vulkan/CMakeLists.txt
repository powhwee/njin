set(SOURCES src/query.cpp
    src/util.cpp
    src/Instance.cpp
    src/Window.cpp
    src/Surface.cpp
    src/PhysicalDevice.cpp
    src/LogicalDevice.cpp
    src/Swapchain.cpp
    src/ImageView.cpp
    src/GraphicsPipeline.cpp
    src/pipeline_util.cpp
    src/ShaderModule.cpp
    src/spirv.cpp
    src/RenderPass.cpp
    src/PipelineLayout.cpp
    src/Buffer.cpp
    src/FramebufferSet.cpp
    src/CommandPool.cpp
    src/CommandBuffer.cpp
    src/Semaphore.cpp
    src/Fence.cpp
    src/GraphicsPipelineBuilder.cpp
    src/DescriptorSetLayoutBuilder.cpp
    src/PipelineLayoutBuilder.cpp
    src/DescriptorSetLayout.cpp
    src/DescriptorPoolBuilder.cpp
    src/DescriptorPool.cpp
    src/Image.cpp
    src/ImageBuilder.cpp
    src/DescriptorSet.cpp
    src/SamplerBuilder.cpp
    src/Sampler.cpp
    src/image_setup.cpp
    src/DescriptorSets.cpp
    src/AttachmentImages.cpp
    src/SubpassModule.cpp
    src/RenderResources.cpp
    src/RenderPasses.cpp
    src/GraphicsPipelines.cpp
    src/Renderer.cpp
    src/VertexBuffers.cpp
    src/FramebufferSet.cpp
    src/FramebufferSets.cpp
    src/RenderInfos.cpp
)

add_library(vulkan STATIC ${SOURCES})
target_include_directories(vulkan PUBLIC include)
target_link_libraries(vulkan PRIVATE
                      Vulkan::Vulkan
                      SDL3::SDL3
                      math
                      core)