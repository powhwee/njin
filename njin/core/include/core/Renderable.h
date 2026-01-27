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
        float base_color_r{ 1.0f };  // Default to white
        float base_color_g{ 1.0f };
        float base_color_b{ 1.0f };
        float base_color_a{ 1.0f };
    };

    struct Renderable {
        RenderType type{ RenderType::Mesh };
        std::variant<MeshData> data;
    };

}  // namespace njin::core