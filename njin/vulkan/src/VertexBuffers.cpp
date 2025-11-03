#include "vulkan/VertexBuffers.h"

namespace njin::vulkan {
    VertexBuffers::VertexBuffers(const LogicalDevice& device,
                                 const PhysicalDevice& physical_device,
                                 const std::vector<VertexBufferInfo>&
                                 buffer_infos) {
        for (const VertexBufferInfo& buffer_info : buffer_infos) {
            BufferSettings settings{
                .size = buffer_info.vertex_input.vertex_size *
                        buffer_info.max_vertex_count,
                .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
            };
            printf("Creating vertex buffer: %s, vertex_size: %d, max_vertex_count: %d, calculated size: %d\n", buffer_info.name.c_str(), buffer_info.vertex_input.vertex_size, buffer_info.max_vertex_count, settings.size);
            Buffer buffer{ device, physical_device, settings };
            buffers_.emplace(buffer_info.name, std::move(buffer));
        };
    }

    VkBuffer VertexBuffers::get_vertex_buffer(const std::string& name) {
        return buffers_.at(name).get();
    }
}  // namespace njin::vulkan