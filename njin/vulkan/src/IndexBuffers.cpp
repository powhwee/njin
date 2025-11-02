#include "vulkan/IndexBuffers.h"

namespace njin::vulkan {
    IndexBuffers::IndexBuffers(const LogicalDevice& device,
                                 const PhysicalDevice& physical_device,
                                 const std::vector<IndexBufferInfo>&
                                 buffer_infos) {
        for (const IndexBufferInfo& buffer_info : buffer_infos) {
            BufferSettings settings{
                .size = sizeof(uint32_t) *
                        buffer_info.max_index_count,
                .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            };
            Buffer buffer{ device, physical_device, settings };
            buffers_.emplace(buffer_info.name, std::move(buffer));
        };
    }

    VkBuffer IndexBuffers::get_index_buffer(const std::string& name) {
        return buffers_.at(name).get();
    }
}  // namespace njin::vulkan
