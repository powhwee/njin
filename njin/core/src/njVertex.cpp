#include "core/njVertex.h"

#include <cstddef>

namespace njin::core {
    njVertex::njVertex(const njVertexCreateInfo& info) :
        position{ info.position.has_value() ? info.position.value() :
                                               math::njVec3f{} },
        normal{ info.normal.has_value() ? info.normal.value() :
                                           math::njVec3f{} },
        tangent{ info.tangent.has_value() ? info.tangent.value() :
                                             math::njVec4f{} },
        tex_coord{ info.tex_coord.has_value() ? info.tex_coord.value() :
                                                 math::njVec2f{} },
        color{ info.color.has_value() ? info.color.value() :
                                         math::njVec4<uint16_t>{} } {}

    njVertex::njVertex(const math::njVec3f& position) : position{ position } {}

    VkVertexInputBindingDescription njVertex::get_binding_description() {
        VkVertexInputBindingDescription binding_description{
            .binding = 0,
            .stride = sizeof(njVertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        return binding_description;
    }

    std::vector<VkVertexInputAttributeDescription>
    njVertex::get_attribute_descriptions() {
        VkVertexInputAttributeDescription position{
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(njVertex, position)
        };

        VkVertexInputAttributeDescription normal{
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(njVertex, normal)
        };

        VkVertexInputAttributeDescription tangent{
            .location = 2,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32A32_SFLOAT,
            .offset = offsetof(njVertex, tangent)
        };

        VkVertexInputAttributeDescription tex_coord{
            .location = 3,
            .binding = 0,
            .format = VK_FORMAT_R32G32_SFLOAT,
            .offset = offsetof(njVertex, tex_coord)
        };

        VkVertexInputAttributeDescription color{
            .location = 4,
            .binding = 0,
            .format = VK_FORMAT_R16G16B16A16_UNORM,
            .offset = offsetof(njVertex, color)
        };

        return { position, normal, tangent, tex_coord, color };
    }

    size_t njVertex::get_size() {
        return sizeof(njVertex);
    }
}  // namespace njin::core