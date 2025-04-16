#include "core/njScene.h"

#include <catch2/catch_test_macros.hpp>
#include <core/loader.h>

namespace njin::core {

    TEST_CASE("njScene", "[core][njScene]") {
        // camera infos
        njRegistry<njCamera> camera_registry{};  // only one
        load_cameras("njScene/valid.cameras", camera_registry);
        njScene::njCameraBindInfo valid_camera_0{ .camera_name = "perspective",
                                                  .node_name = "root" };
        njScene::njCameraBindInfo invalid_camera_0{
            .camera_name = "non-existent camera",
            .node_name = "root"
        };

        std::vector<njScene::njCameraBindInfo> valid_camera_infos{
            valid_camera_0
        };
        std::vector<njScene::njCameraBindInfo> invalid_camera_infos{
            valid_camera_0,
            invalid_camera_0
        };

        // mesh infos
        njRegistry<njMesh> mesh_registry{};
        njRegistry<njMesh> empty_mesh_registry{};
        load_meshes("njScene/valid.meshes", mesh_registry);
        njScene::njMeshBindInfo valid_mesh_0{ .mesh_name = "cube",
                                              .node_name = "node_0" };
        njScene::njMeshBindInfo valid_mesh_1{ .mesh_name = "sphere",
                                              .node_name = "node_1" };
        njScene::njMeshBindInfo invalid_mesh_0{
            .mesh_name = "non-existent mesh",
            .node_name = "node_0"
        };
        std::vector<njScene::njMeshBindInfo> valid_mesh_infos{ valid_mesh_0,
                                                               valid_mesh_1 };

        std::vector<njScene::njMeshBindInfo> invalid_mesh_infos{
            valid_mesh_0,
            invalid_mesh_0
        };

        // node infos

        njScene::njNodeInfo root{ .name = "root",
                                  .transform = {},
                                  .parent_name = std::nullopt };
        njScene::njNodeInfo valid_node_0{
            .name = "valid_node_0",
            .transform = {},
            .parent_name = std::optional("root")
        };
        njScene::njNodeInfo valid_node_1{
            .name = "valid_node_1",
            .transform = {},
            .parent_name = std::optional("valid_node_0")
        };

        njScene::njNodeInfo invalid_node_0{
            .name = "invalid_node_0",
            .transform = {},
            .parent_name = std::optional("non-existent node name")
        };

        std::vector<njScene::njNodeInfo> valid_node_infos{ root,
                                                           valid_node_0,
                                                           valid_node_1 };

        std::vector<njScene::njNodeInfo> invalid_node_infos{ root,
                                                             valid_node_0,
                                                             valid_node_1,
                                                             invalid_node_0 };

        SECTION("construction") {
            SECTION("using camera that does not exist in registry") {
                std::vector<njScene::njCameraBindInfo> camera_infos{
                    { "blah", "root" }
                };
                REQUIRE_THROWS(njScene{ {},
                                        camera_infos,
                                        {},
                                        camera_registry,
                                        empty_mesh_registry });
            }
            SECTION("using camera that does exist in registry") {
                std::vector<njScene::njCameraBindInfo> camera_infos{
                    { "perspective", "root" }
                };

                REQUIRE_NOTHROW(njScene{ {},
                                         camera_infos,
                                         {},
                                         camera_registry,
                                         empty_mesh_registry });
            }

            SECTION("referencing non-existent nodes") {
                REQUIRE_THROWS(njScene{ invalid_node_infos,
                                        valid_camera_infos,
                                        valid_mesh_infos,
                                        camera_registry,
                                        mesh_registry });
            }

            SECTION("valid node infos") {
                REQUIRE_NOTHROW(njScene{ valid_node_infos,
                                         valid_camera_infos,
                                         valid_mesh_infos,
                                         camera_registry,
                                         mesh_registry });
            }

            SECTION("referencing non-existent meshes") {
                REQUIRE_THROWS(njScene{ valid_node_infos,
                                        valid_camera_infos,
                                        invalid_mesh_infos,
                                        camera_registry,
                                        mesh_registry });
            }

            SECTION("valid mesh infos") {
                REQUIRE_NOTHROW(njScene{ valid_node_infos,
                                         valid_camera_infos,
                                         valid_mesh_infos,
                                         camera_registry,
                                         mesh_registry });
            }
        }

        SECTION("scene verification") {
            const njScene scene{ valid_node_infos,
                                 valid_camera_infos,
                                 valid_mesh_infos,
                                 camera_registry,
                                 mesh_registry };

            REQUIRE_NOTHROW(scene.get_root());
            njNode* root_node{ scene.get_root() };
            REQUIRE(root_node != nullptr);
            REQUIRE(root_node->get_name() == "root");
            std::vector<njNode*> root_children{ root_node->get_children() };
            REQUIRE(root_children.size() == 1);
            REQUIRE(root_children.at(0)->get_name() == "valid_node_0");
            njNode* node_0{ root_children.at(0) };
            std::vector<njNode*> node_0_children{ node_0->get_children() };
            REQUIRE(node_0_children.size() == 1);
            REQUIRE(node_0_children.at(0)->get_name() == "valid_node_1");
            njNode* node_1{ node_0_children.at(0) };
            std::vector<njNode*> node_1_children{ node_1->get_children() };
            REQUIRE(node_1_children.size() == 0);
        }
    }

}  // namespace njin::core
