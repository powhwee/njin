#pragma once
#include <string>
#include <vector>

#include "math/njVec4.h"

namespace njin::core {

    /**
     * Represents a material, typically loaded from a glTF file.
     */
    struct njMaterial {
        std::string name;
        math::njVec4f base_color_factor{ 1.0f, 1.0f, 1.0f, 1.0f }; // Default to white opaque
        // Name of the base color texture (empty string indicates no texture)
        std::string base_color_texture_name;
    };

}  // namespace njin::core

