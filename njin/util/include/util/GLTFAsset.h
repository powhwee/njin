#pragma once
#include <string>
#include <vector>

#include "core/njMesh.h"
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
        */
        GLTFAsset(const std::string& path);

        /**
         * Retrieve the array of meshes from this asset
         * @return Array of meshes
         */
        std::vector<core::njMesh> get_meshes() const;

        private:
        uint32_t length_{ 0 };
        gltf::Buffer buffer_{};
        std::vector<gltf::BufferView> buffer_views_{};
        std::vector<core::njMesh> meshes_{};
    };
}  // namespace njin::gltf
