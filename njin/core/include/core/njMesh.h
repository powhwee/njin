#pragma once
#include <vector>

#include "core/njPrimitive.h"
#include "core/njVertex.h"
#include "math/njVec3.h"

namespace njin::core {

    struct njMeshCreateInfo {
        std::string name;
        std::vector<njPrimitive> primitives;
        math::njVec3f min_bounds;
        math::njVec3f max_bounds;
    };

    class njMesh {
        public:
        std::string name;
        math::njVec3f min_bounds;
        math::njVec3f max_bounds;

        njMesh() = default;
        njMesh(const njMeshCreateInfo& info);

        /**
         * Get the list of all vertices in this mesh
         * @return List of all vertices
         */
        std::vector<njVertex> get_vertices() const;

        /**
         * Get the list of all primitives in this mesh
         * @return List of primitives
         */
        std::vector<njPrimitive> get_primitives() const;

        /**
         * Get the amount of memory in bytes this mesh's vertices take up
         * @return Size of mesh in bytes
         */
        int get_size() const;

        /**
         * Get the total number of vertices in the mesh across all primitives
         * @return Number of vertices
         */
        uint32_t get_vertex_count() const;

        private:
        std::vector<njPrimitive> primitives_{};
    };

}  // namespace njin::core
