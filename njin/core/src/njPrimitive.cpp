#include "core/njPrimitive.h"

namespace njin::core {

    njPrimitive::njPrimitive(const std::vector<njVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& material_name) :
        vertices_{ vertices }, indices_{ indices }, material_name_{ material_name } {}

    std::vector<njVertex> njPrimitive::get_vertices() const {
        return vertices_;
    }

    std::vector<uint32_t> njPrimitive::get_indices() const {
        return indices_;
    }

    std::string njPrimitive::get_material_name() const {
        return material_name_;
    }
}  // namespace njin::core