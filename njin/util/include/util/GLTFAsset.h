#pragma once
#include <string>
#include <vector>

#include "core/njMesh.h"
#include "core/njMaterial.h"
#include "core/njTexture.h"
#include "util/Buffer.h"
#include "util/BufferView.h"

namespace njin::gltf {
    /**
     * Representation of a GLTF asset, parsed from a given file
     */
    class GLTFAsset {
        public:
        /**
         * Constructor
         * @param path Path to GLB file
         * @param alias Namespace alias for prefixing materials, textures, and mesh material references
         */
        GLTFAsset(const std::string& path, const std::string& alias);

        /**
         * Retrieve the array of meshes from this asset
         * @return Array of meshes
         */
        std::vector<core::njMesh> get_meshes() const;

        /**
         * Retrieve the array of materials from this asset
         * @return Array of materials
         */
        std::vector<core::njMaterial> get_materials() const;

        /**
         * Retrieve the array of textures from this asset
         * @return Array of textures
         */
        std::vector<core::njTexture> get_textures() const;

        private:
        uint32_t length_{ 0 };
        std::string alias_;
        gltf::Buffer buffer_{};
        std::vector<gltf::BufferView> buffer_views_{};
        std::vector<core::njMesh> meshes_{};
        std::vector<core::njMaterial> materials_{};
        std::vector<core::njTexture> textures_{};
    };
}  // namespace njin::gltf

