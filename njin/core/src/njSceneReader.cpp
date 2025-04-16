#include "core/njSceneReader.h"

#include "math/njVec4.h"
#include "util/file.h"
#include "util/json.h"

#include <set>

namespace rj = rapidjson;

namespace {
    namespace core = njin::core;
    namespace math = njin::math;

    /**
     * Add all nodes in the .scene file to the hierarchy
     * @param document rapidjson document representation of the scene file
     * @return List of njNodeInfo that the scene will use to construct
     * its hierarchy
     */
    using njNodeInfo = core::njScene::njNodeInfo;

    std::vector<njNodeInfo> process_nodes(rj::Document& document) {
        rj::GenericArray nodes{ document["nodes"].GetArray() };

        std::vector<njNodeInfo> node_infos{};
        for (const auto& rj_node : nodes) {
            std::string name{ rj_node["name"].GetString() };

            rj::GenericArray rj_transform{ rj_node["transform"].GetArray() };
            math::njVec4f row_0{ rj_transform[0][0].GetFloat(),
                                 rj_transform[0][1].GetFloat(),
                                 rj_transform[0][2].GetFloat(),
                                 rj_transform[0][3].GetFloat() };

            math::njVec4f row_1{ rj_transform[1][0].GetFloat(),
                                 rj_transform[1][1].GetFloat(),
                                 rj_transform[1][2].GetFloat(),
                                 rj_transform[1][3].GetFloat() };

            math::njVec4f row_2{ rj_transform[2][0].GetFloat(),
                                 rj_transform[2][1].GetFloat(),
                                 rj_transform[2][2].GetFloat(),
                                 rj_transform[2][3].GetFloat() };

            math::njVec4f row_3{ rj_transform[3][0].GetFloat(),
                                 rj_transform[3][1].GetFloat(),
                                 rj_transform[3][2].GetFloat(),
                                 rj_transform[3][3].GetFloat() };

            math::njMat4f transform{ row_0, row_1, row_2, row_3 };

            std::optional<std::string> parent_name{};
            if (rj_node.HasMember("parent")) {
                parent_name = rj_node["parent"].GetString();
            }

            njNodeInfo info{ .name = name,
                             .transform = transform,
                             .parent_name = parent_name };
            node_infos.push_back(info);
        }
        return node_infos;
    }

    /**
     * Create a list of njCameraInfo for njScene to use to set up
     * the scene's cameras
     * @param document rapidjson document representation of the .scene file
     * @return List of njCameraInfo
     */
    using njCameraInfo = core::njScene::njCameraBindInfo;

    std::vector<njCameraInfo> process_cameras(rj::Document& document) {
        rj::GenericArray cameras{ document["cameras"].GetArray() };
        std::vector<njCameraInfo> camera_infos{};
        for (auto& rj_camera : cameras) {
            std::string camera_name{ rj_camera["name"].GetString() };
            std::string node_name{ rj_camera["node"].GetString() };
            njCameraInfo info{ .camera_name = camera_name,
                               .node_name = node_name };
            camera_infos.push_back(info);
        }

        return camera_infos;
    }

    /**
     * Create a list of njMeshInfo for njScene to use to set up
     * the scene's meshes
     * @param document rapidjson document representation of the .scene file
     * @return List of njMeshInfo
     */
    using njMeshInfo = core::njScene::njMeshBindInfo;

    std::vector<njMeshInfo> process_meshes(rj::Document& document) {
        std::vector<njMeshInfo> mesh_infos{};
        if (!document
             .HasMember("meshes")) {  // .scene file might not have meshes
            return {};
        }
        for (auto& rj_mesh : document["meshes"].GetArray()) {
            const std::string mesh_name{ rj_mesh["mesh"].GetString() };
            const std::string node_name{ rj_mesh["node"].GetString() };
            njMeshInfo info{ .mesh_name = mesh_name, .node_name = node_name };
            mesh_infos.push_back(info);
        }

        return mesh_infos;
    }
}  // namespace

namespace njin::core {
    njSceneReader::njSceneReader(const std::string& path,
                                 const njRegistry<njCamera>& camera_registry,
                                 const njRegistry<njMesh>& mesh_registry) {
        rj::Document document{
            util::make_validated_document("schema/scene.schema.json", path)
        };

        std::vector<njScene::njNodeInfo> node_infos{ process_nodes(document) };
        std::vector<njScene::njCameraBindInfo> camera_infos{
            process_cameras(document)
        };
        std::vector<njScene::njMeshBindInfo> mesh_infos{ process_meshes(document) };

        scene_ = { node_infos,
                   camera_infos,
                   mesh_infos,
                   camera_registry,
                   mesh_registry };
    }

    njScene njSceneReader::get_scene() const {
        return scene_;
    }
}  // namespace njin::core
