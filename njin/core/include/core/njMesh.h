#pragma once
#include <vector>

#include "core/njPrimitive.h"
#include "core/njVertex.h"

namespace njin::core {

    class njMesh {
        public:
        std::string name;

        njMesh() = default;
        njMesh(const std::string& name, const std::vector<njPrimitive>& primitives);

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
