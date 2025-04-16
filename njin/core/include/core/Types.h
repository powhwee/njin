#pragma once

#include <string>
#include <unordered_map>

// vertex offset + length of the mesh in the vertex buffer
using MeshInfo = std::pair<uint32_t, uint32_t>;
// map of mesh names to the corresponding MeshInfo
using MeshIndices = std::unordered_map<std::string, MeshInfo>;

// map of texture names to the index of the sampler in the texture descriptor array
using TextureIndices = std::unordered_map<std::string, uint32_t>;

enum class RenderType : uint8_t {
    Mesh,
    Wireframe
};
