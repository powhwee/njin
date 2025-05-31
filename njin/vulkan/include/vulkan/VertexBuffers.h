#pragma once
#include <unordered_map>
#include <vector>

#include "vulkan/Buffer.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    class VertexBuffers {
        public:
        VertexBuffers(const LogicalDevice& device,
                      const PhysicalDevice& physical_device,
                      const std::vector<VertexBufferInfo>& buffer_infos);

        VkBuffer get_vertex_buffer(const std::string& name);

        template<typename T>
        void load_into_buffer(const std::string& name,
                              const std::vector<T>& data) {
            Buffer& buffer{ buffers_.at(name) };
            buffer.load(data);
        }

        private:
        std::unordered_map<std::string, Buffer> buffers_{};
    };
}  // namespace njin::vulkan