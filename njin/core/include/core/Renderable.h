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
    };

    struct WireframeData {
        math::njMat4f global_transform;
        std::vector<njVertex> line_list{};
    };

    struct Renderable {
        RenderType type{ RenderType::Mesh };
        std::variant<MeshData, WireframeData> data;
    };

}  // namespace njin::core