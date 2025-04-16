#pragma once
#include <optional>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "math/njVec2.h"
#include "math/njVec3.h"
#include "math/njVec4.h"

namespace njin::core {
    struct njVertexCreateInfo {
        std::optional<math::njVec3f> position{};
        std::optional<math::njVec4<uint16_t>> color{};
        std::optional<math::njVec3f> normal{};
        std::optional<math::njVec4f> tangent{};
        std::optional<math::njVec2f> tex_coord{};
    };

    /**
     * Contains information about one vertex and its attributes
     * Attributes stored within: position, color, normal, tangent, texture coordinate
     */
    class njVertex {
        public:
        explicit njVertex(const njVertexCreateInfo& info);

        njVertex(const math::njVec3f& position);

        /**
         * Get the vertex input binding description for an njVertex
         * @return Vertex input binding description
         * @note For use with vulkan only
         */
        static VkVertexInputBindingDescription get_binding_description();

        /**
         * Get a list of vertex input attribute descriptions for an njVertex
         * @return List of vertex input attribute descriptions
         */
        static std::vector<VkVertexInputAttributeDescription>
        get_attribute_descriptions();

        /**
         * Get the size in bytes of a vertex
         * @return Size in bytes of an njVertex
         */
        static size_t get_size();

        private:
        math::njVec3f position_{};
        math::njVec3f normal_{};
        math::njVec4f tangent_{};
        math::njVec2f tex_coord_{};
        math::njVec4<uint16_t> color_{};
    };

    static_assert(std::is_standard_layout_v<njVertex>,
                  "njVertex must be standard layout");
}  // namespace njin::core