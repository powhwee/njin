#include "core/njMesh.h"

namespace njin::core {

    njMesh::njMesh(const std::string& name, const std::vector<njPrimitive>& primitives) :
        name{ name }, primitives_{ primitives } {}

    std::vector<njVertex> njMesh::get_vertices() const {
        std::vector<njVertex> vertices{};
        for (const auto& primitive : primitives_) {
            std::vector<njVertex> primitive_vertices{
                primitive.get_vertices()
            };
            vertices.insert(vertices.end(),
                            primitive_vertices.begin(),
                            primitive_vertices.end());
        }

        return vertices;
    }

    std::vector<njPrimitive> njMesh::get_primitives() const {
        return primitives_;
    }

    int njMesh::get_size() const {
        int size = 0;
        for (const auto& primitive : primitives_) {
            size += primitive.get_vertices().size() * sizeof(njVertex);
        }
        return size;
    }

    uint32_t njMesh::get_vertex_count() const {
        uint32_t count = 0;
        for (const auto& primitive : primitives_) {
            count += primitive.get_vertices().size();
        }
        return count;
    }

}  // namespace njin::core