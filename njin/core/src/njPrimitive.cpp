#include "core/njPrimitive.h"

namespace njin::core {

    njPrimitive::njPrimitive(const std::vector<njVertex>& vertices, const std::vector<uint32_t>& indices) :
        vertices_{ vertices }, indices_{ indices } {}

    std::vector<njVertex> njPrimitive::get_vertices() const {
        return vertices_;
    }

    std::vector<uint32_t> njPrimitive::get_indices() const {
        return indices_;
    }
}  // namespace njin::core