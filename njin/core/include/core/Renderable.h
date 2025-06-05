#pragma once
#include <variant>
#include <vector>

#include "core/Types.h"
#include "core/njVertex.h"
#include "math/njMat4.h"

namespace njin::core {
    struct MeshData {
        math::njMat4f global_transform;
        std::string mesh_name;
        std::string texture_name;
    };

    struct Renderable {
        RenderType type{ RenderType::Mesh };
        std::variant<MeshData> data;
    };

}  // namespace njin::core