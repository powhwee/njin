#include "core/njScene.h"

namespace njin::core {
    njScene::njScene(const std::vector<njNodeInfo>& node_infos,
                     const std::vector<njCameraBindInfo>& camera_infos,
                     const std::vector<njMeshBindInfo>& mesh_infos,
                     const njRegistry<njCamera>& camera_registry,
                     const njRegistry<njMesh>& mesh_registry) {
        setup_nodes(node_infos);
        setup_cameras(camera_infos, camera_registry);
        setup_meshes(mesh_infos, mesh_registry);
    }

    void njScene::setup_nodes(const std::vector<njNodeInfo>& node_infos) {
        // make all the nodes and put them in the node store first
        for (const njNodeInfo& node_info : node_infos) {
            njNode node{ node_info.name, node_info.transform };
            nodes_.add(node_info.name, node);
        }

        // then set up the hierarchy: for nodes that specify a parent (via a name)
        // check if a node with that name really exists and then parent to that node
        for (const njNodeInfo& node_info : node_infos) {
            if (!node_info.parent_name.has_value()) {
                continue;
            }
            std::string parent_name{ node_info.parent_name.value() };
            njNode* parent{ nodes_.get(parent_name) };
            if (!parent) {
                throw std::runtime_error(
                std::format("Node '{}' does not exist!", parent_name));
            }
            njNode* child{ nodes_.get(node_info.name) };  // this node
            parent->add_child(child);
        }

        root_node_ = nodes_.get("root");
    }

    void
    njScene::setup_cameras(const std::vector<njCameraBindInfo>& camera_infos,
                           const njRegistry<njCamera>& camera_registry) {
        bool first{ true };
        for (const njCameraBindInfo& info : camera_infos) {
            const njCamera* global_camera{
                camera_registry.get(info.camera_name)
            };
            if (!global_camera) {
                throw std::runtime_error(
                std::format("Camera '{}' not in registry", info.camera_name));
            }
            // make local copy (local to the scene) and add to local camera
            // registry
            njCamera copy{ *global_camera };
            cameras_.add(info.camera_name, copy);

            // work with that local copy
            njCamera* camera{ cameras_.get(info.camera_name) };
            const njNode* target{ nodes_.get(info.node_name) };
            camera->set_node(target);
            // the first camera in the camera array in .scene is the default
            // active camera
            if (first) {
                active_camera_ = camera;
                first = false;
            }
        }
    }

    void njScene::setup_meshes(const std::vector<njMeshBindInfo>& mesh_infos,
                               const njRegistry<njMesh>& mesh_registry) {
        for (const njMeshBindInfo& info : mesh_infos) {
            const njMesh* global_mesh{ mesh_registry.get(info.mesh_name) };
            if (!global_mesh) {
                throw std::runtime_error(
                std::format("Mesh '{}' not in registry", info.mesh_name));
            }
            njMesh copy{ *global_mesh };
            meshes_.add(info.mesh_name, copy);

            // work with local copy
            njMesh* mesh{ meshes_.get(info.mesh_name) };
            njNode* target_node{ nodes_.get(info.node_name) };
        }
    }

    njNode* njScene::get_root() const {
        return root_node_;
    }

    const njCamera* njScene::get_active_camera() const {
        active_camera_->look_at({ 0, 0, 0 }, { 0, 1, 0 });
        return active_camera_;
    }

    std::vector<const njMesh*> njScene::get_meshes() const {
        return meshes_.get_records();
    }
}  // namespace njin::core