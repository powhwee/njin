#pragma once
#include <vector>

#include "core/njVertex.h"
#include "math/njMat4.h"

namespace njin::core {
    enum class RenderableType : uint8_t {
        Mesh,
        Wireframe
    };

    // Wireframe type: mesh_name and texture_name are ignored
    // Mesh type: line list is ignored
    struct Renderable {
        RenderableType type{ RenderableType::Mesh };
        math::njMat4f global_transform;
        std::string mesh_name;
        std::string texture_name;
        // list of lines that form the wireframe. The coordinates
        // are in local space
        std::vector<njVertex> line_list;
    };

}  // namespace njin::core