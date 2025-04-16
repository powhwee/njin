#pragma once
#include <vector>

#include "core/njCamera.h"
#include "core/njMesh.h"
#include "core/njNode.h"
#include "core/njRegistry.h"

namespace njin::core {
    /**
    * Manages the node hierarchy and stuff that references the nodes
    */
    class njScene {
        public:
        struct njNodeInfo {
            std::string name{};
            math::njMat4f transform{};
            std::optional<std::string> parent_name{};
        };

        struct njCameraBindInfo {
            std::string camera_name{};  // name of camera asset
            std::string node_name{};    // name of node to attach this camera to
        };

        struct njMeshBindInfo {
            std::string mesh_name{};  // name of mesh asset
            std::string node_name{};  // name of node to attach this mesh to
        };

        njScene() = default;
        njScene(const std::vector<njNodeInfo>& node_infos,
                const std::vector<njCameraBindInfo>& camera_infos,
                const std::vector<njMeshBindInfo>& mesh_infos,
                const njRegistry<njCamera>& camera_registry,
                const njRegistry<njMesh>& mesh_registry);

        njNode* get_root() const;

        /**
         * Retrieve the active camera
         * @return Active camera
         */
        const njCamera* get_active_camera() const;

        /**
         * Retrieve the list of meshes in the scene
         * @return List of meshes in the scene
         */
        std::vector<const njMesh*> get_meshes() const;

        private:
        njRegistry<njNode> nodes_{};
        njRegistry<njCamera> cameras_{};
        njRegistry<njMesh> meshes_{};

        /**
         * Populate scene's node registry and set the root node
         * @param node_infos List of node infos
         */
        void setup_nodes(const std::vector<njNodeInfo>& node_infos);

        /**
         * Populate scene's camera registry and set the active camera
         * @param camera_infos List of camera infos
         * @param camera_registry Global camera registry
         */
        void setup_cameras(const std::vector<njCameraBindInfo>& camera_infos,
                           const njRegistry<njCamera>& camera_registry);

        /**
         * Populate scene's mesh registry
         * @param mesh_infos List of mesh infos
         * @param mesh_registry Global mesh registry
         */
        void setup_meshes(const std::vector<njMeshBindInfo>& mesh_infos,
                          const njRegistry<njMesh>& mesh_registry);

        njCamera* active_camera_{ nullptr };
        njNode* root_node_{ nullptr };
    };
}  // namespace njin::core