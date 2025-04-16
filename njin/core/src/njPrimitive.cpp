#include "core/njPrimitive.h"

namespace njin::core {

    njPrimitive::njPrimitive(const std::array<njVertex, 3>& vertices) :
        vertices_{ vertices } {}

    std::vector<njVertex> njPrimitive::get_vertices() const {
        return std::vector<njVertex>(vertices_.begin(), vertices_.end());
    }
}  // namespace njin::core