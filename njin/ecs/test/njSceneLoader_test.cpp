#include <catch2/catch_test_macros.hpp>
#include "ecs/njSceneLoader.h"
#include "ecs/njEngine.h"
#include "ecs/njObjectArchetype.h"
#include "ecs/njPlayerArchetype.h"
#include "ecs/Components.h"
#include "core/njRegistry.h"
#include "core/njMesh.h"
#include "util/json.h"
#include <fstream>
#include <cstdio>

namespace njin::ecs {

// RAII helper to clean up test files
struct TestFileGuard {
    std::string path;
    ~TestFileGuard() { std::remove(path.c_str()); }
};

TEST_CASE("njSceneLoader", "[ecs][scene_loader]") {
    // Setup registries
    core::njRegistry<core::njMesh> mesh_registry;
    core::njRegistry<core::njMaterial> material_registry;
    core::njRegistry<core::njTexture> texture_registry;
    
    // Add dummy meshes
    core::njMesh cube_mesh;
    cube_mesh.name = "cube";
    mesh_registry.add("cube-Object_0", std::move(cube_mesh));
    
    core::njMesh player_mesh;
    player_mesh.name = "player";
    mesh_registry.add("player-Cube", std::move(player_mesh));

    SECTION("load full scene creates all entities") {
        std::string scene_path = "test_full_scene.json";
        TestFileGuard guard{scene_path};
        
        std::ofstream file(scene_path);
        file << R"({
            "camera": {
                "name": "main_camera",
                "position": [0, 5, 10],
                "look_at": [0, 0, 0]
            },
            "entities": [
                {
                    "name": "test_cube",
                    "mesh_alias": "cube",
                    "position": [2, 0, 0],
                    "rotation_x_degrees": 90
                },
                {
                    "name": "test_player",
                    "archetype": "player",
                    "mesh_alias": "player",
                    "position": [0, 1, 0],
                    "physics": {
                        "mass": 10.0,
                        "type": "dynamic"
                    }
                }
            ]
        })";
        file.close();

        njEngine engine;
        njSceneLoader loader(scene_path, mesh_registry, material_registry, texture_registry);
        
        REQUIRE_NOTHROW(loader.load(engine));

        // Verify camera created
        auto camera_views = engine.get_view<njCameraComponent>();
        REQUIRE(camera_views.size() == 1);
        
        // Verify player created (has transform, mesh, input, physics)
        auto player_views = engine.get_view<njTransformComponent, njInputComponent, njPhysicsComponent>();
        REQUIRE(player_views.size() == 1);
        
        auto [transform, input, physics] = player_views[0].second;
        REQUIRE(physics->mass == 10.0f);
        REQUIRE(physics->type == RigidBodyType::Dynamic);

        // Verify mesh count (player + cube = 2)
        auto mesh_views = engine.get_view<njMeshComponent>();
        REQUIRE(mesh_views.size() == 2);
    }
    
    SECTION("load camera-only scene") {
        std::string scene_path = "test_camera_only.json";
        TestFileGuard guard{scene_path};
        
        std::ofstream file(scene_path);
        file << R"({
            "camera": {
                "name": "solo_camera",
                "position": [0, 10, 0],
                "look_at": [0, 0, 0]
            }
        })";
        file.close();

        njEngine engine;
        njSceneLoader loader(scene_path, mesh_registry, material_registry, texture_registry);
        
        REQUIRE_NOTHROW(loader.load(engine));

        auto camera_views = engine.get_view<njCameraComponent>();
        REQUIRE(camera_views.size() == 1);
        
        auto mesh_views = engine.get_view<njMeshComponent>();
        REQUIRE(mesh_views.size() == 0);
    }
    
    SECTION("load scene with empty entities array") {
        std::string scene_path = "test_empty_entities.json";
        TestFileGuard guard{scene_path};
        
        std::ofstream file(scene_path);
        file << R"({
            "camera": {
                "name": "empty_scene_camera",
                "position": [0, 5, 10],
                "look_at": [0, 0, 0]
            },
            "entities": []
        })";
        file.close();

        njEngine engine;
        njSceneLoader loader(scene_path, mesh_registry, material_registry, texture_registry);
        
        REQUIRE_NOTHROW(loader.load(engine));

        auto camera_views = engine.get_view<njCameraComponent>();
        REQUIRE(camera_views.size() == 1);
        
        auto mesh_views = engine.get_view<njMeshComponent>();
        REQUIRE(mesh_views.size() == 0);
    }
}

}  // namespace njin::ecs
