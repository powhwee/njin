#pragma once
#include <string>
#include <vector>

#include "core/njAnimation.h"
#include "core/njMaterial.h"
#include "core/njMesh.h"
#include "core/njSkeleton.h"
#include "core/njTexture.h"
#include "math/njMat4.h"
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

        /**
         * Retrieve the animations from this asset
         * @return Array of animations
         */
        std::vector<core::njAnimation> get_animations() const;

        /**
         * Retrieve the skeleton (node hierarchy) from this asset
         * @return Skeleton
         */
        core::njSkeleton get_skeleton() const;

        private:

        private:
        // Internal node struct removal - we now use njSkeleton

        void
        process_node_hierarchy(int node_index,
                               int parent_node_index,
                               const std::vector<core::njSkeletonNode>& nodes);

        uint32_t length_{ 0 };
        std::string alias_;
        gltf::Buffer buffer_{};
        std::vector<gltf::BufferView> buffer_views_{};
        std::vector<core::njMesh> meshes_{};  // Raw meshes from the file
        std::vector<core::njMesh>
        processed_meshes_{};  // Meshes with transforms baked in
        std::vector<core::njMaterial> materials_{};
        std::vector<core::njTexture> textures_{};
        std::vector<core::njAnimation> animations_{};
        core::njSkeleton skeleton_{};
    };
}  // namespace njin::gltf
