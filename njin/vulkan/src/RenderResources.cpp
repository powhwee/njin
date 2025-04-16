#include "vulkan/RenderResources.h"

namespace njin::vulkan {
    RenderResources::RenderResources(const LogicalDevice& logical_device,
                                     const PhysicalDevice& physical_device,
                                     Swapchain& swapchain,
                                     const RenderResourceInfos& infos) :
        descriptor_sets{ logical_device, physical_device, infos.set_layouts },
        attachment_images{ logical_device,
                           physical_device,
                           swapchain,
                           infos.attachment_images },
        render_passes{ logical_device, infos.render_passes, attachment_images },
        graphics_pipelines{ logical_device,
                            render_passes,
                            descriptor_sets,
                            infos.pipelines },
        vertex_buffers{ logical_device, physical_device, infos.vertex_buffers },
        framebuffers{ logical_device, render_passes, attachment_images },
        command_pool_{ logical_device,
                       physical_device.get_graphics_family_index() } {}

    CommandPool& RenderResources::get_command_pool() {
        return command_pool_;
    }

}  // namespace njin::vulkan
