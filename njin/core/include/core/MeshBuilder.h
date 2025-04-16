#pragma once

#include <vector>

#include "core/njMesh.h"
#include "math/njVec3.h"
#include "math/njVec4.h"

namespace njin::core {
    /**
    * Assembles njMeshes based on provided attribute data
    * Assumes that the length of each provided attribute array is the exact same
    * This is as required by the glTF spec: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#meshes
    * "All attribute accessors for a given primitive MUST have the same count"
    */
    class MeshBuilder {
        public:
        /**
         * Constructor
         * @param indices Vertex indices for this mesh, to be interpreted
         * as a triangle list (i.e., every 3 indices is a triangle)
         */
        explicit MeshBuilder(const std::vector<uint16_t>& indices);

        void
        add_position_attributes(const std::vector<math::njVec3f>& positions);

        void add_normal_attributes(const std::vector<math::njVec3f>& normals);

        void add_tangent_attributes(const std::vector<math::njVec4f>& tangents);

        void
        add_color_attributes(const std::vector<math::njVec4<uint16_t>>& colors);

        void
        add_texture_coordinates(const std::vector<math::njVec2f>& tex_coords);

        njMesh build();

        private:
        std::vector<math::njVec3f> positions_{};
        std::vector<math::njVec3f> normals_{};
        std::vector<math::njVec4f> tangents_{};
        std::vector<math::njVec4<uint16_t>> colors_{};
        std::vector<math::njVec2f> tex_coords_{};
        std::vector<uint16_t> indices_;
    };
}  // namespace njin::core