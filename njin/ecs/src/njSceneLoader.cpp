#include "ecs/njSceneLoader.h"

#include <iostream>
#include <cmath>

#include <rapidjson/document.h>

#include "util/json.h"
#include "ecs/njCameraArchetype.h"
#include "ecs/njObjectArchetype.h"
#include "ecs/njPlayerArchetype.h"
#include "math/njMat4.h"
#include "math/njVec3.h"
#include "math/njVec4.h"

namespace rj = rapidjson;

namespace njin::ecs {

njSceneLoader::njSceneLoader(const std::string& scene_path,
                             const njin::core::njRegistry<njin::core::njMesh>& mesh_registry,
                             const njin::core::njRegistry<njin::core::njMaterial>& material_registry,
                             const njin::core::njRegistry<njin::core::njTexture>& texture_registry) :
    scene_path_{ scene_path },
    mesh_registry_{ &mesh_registry },
    material_registry_{ &material_registry },
    texture_registry_{ &texture_registry } {}

void njSceneLoader::load(njEngine& engine) {
    std::cout << "Loading scene from: " << scene_path_ << std::endl;
    
    rj::Document doc = util::make_document(scene_path_);
    
    // Load camera
    if (doc.HasMember("camera") && doc["camera"].IsObject()) {
        const auto& cam = doc["camera"];
        
        float pos_x = cam["position"][0].GetFloat();
        float pos_y = cam["position"][1].GetFloat();
        float pos_z = cam["position"][2].GetFloat();
        
        float look_x = cam["look_at"][0].GetFloat();
        float look_y = cam["look_at"][1].GetFloat();
        float look_z = cam["look_at"][2].GetFloat();
        
        float up_x = cam.HasMember("up") ? cam["up"][0].GetFloat() : 0.f;
        float up_y = cam.HasMember("up") ? cam["up"][1].GetFloat() : 1.f;
        float up_z = cam.HasMember("up") ? cam["up"][2].GetFloat() : 0.f;
        
        float fov = cam.HasMember("fov") ? cam["fov"].GetFloat() : 90.f;
        float near_plane = cam.HasMember("near") ? cam["near"].GetFloat() : 0.1f;
        float far_plane = cam.HasMember("far") ? cam["far"].GetFloat() : 1000.f;
        float aspect = cam.HasMember("aspect") ? cam["aspect"].GetFloat() : 16.f / 9.f;
        
        std::string name = cam.HasMember("name") ? cam["name"].GetString() : "camera";
        
        ecs::PerspectiveCameraSettings settings{ .near = near_plane, .far = far_plane, .horizontal_fov = fov };
        ecs::njCameraArchetypeCreateInfo camera_info{
            .name = name,
            .transform = ecs::njTransformComponent::make(pos_x, pos_y, pos_z),
            .camera = { .type = ecs::njCameraType::Perspective,
                        .up = { up_x, up_y, up_z },
                        .look_at = { look_x, look_y, look_z },
                        .aspect = aspect,
                        .settings = settings }
        };
        
        ecs::njCameraArchetype camera_archetype{ camera_info };
        engine.add_archetype(camera_archetype);
        std::cout << "Camera '" << name << "' loaded." << std::endl;
    }
    
    // Load entities
    if (doc.HasMember("entities") && doc["entities"].IsArray()) {
        const auto& entities = doc["entities"].GetArray();
        
        for (const auto& entity : entities) {
            std::string name = entity.HasMember("name") ? entity["name"].GetString() : "";
            std::string mesh_alias = entity.HasMember("mesh_alias") ? entity["mesh_alias"].GetString() : "";
            std::string archetype_type = entity.HasMember("archetype") ? entity["archetype"].GetString() : "object";
            
            // Get position
            float pos_x = entity.HasMember("position") ? entity["position"][0].GetFloat() : 0.f;
            float pos_y = entity.HasMember("position") ? entity["position"][1].GetFloat() : 0.f;
            float pos_z = entity.HasMember("position") ? entity["position"][2].GetFloat() : 0.f;
            
            // Build transform with optional rotation
            math::njMat4f translation{ math::njMat4Type::Translation, math::njVec3f{pos_x, pos_y, pos_z} };
            math::njMat4f final_transform = translation;
            
            if (entity.HasMember("rotation_x_degrees")) {
                float angle_x = entity["rotation_x_degrees"].GetFloat() * 3.14159f / 180.f;
                float half_x = angle_x / 2.f;
                math::njVec4f quat_x{ std::sin(half_x), 0.f, 0.f, std::cos(half_x) };
                math::njMat4f rotation_x{ quat_x };
                final_transform = translation * rotation_x;
            }
            
            if (archetype_type == "player") {
                // Player archetype with physics
                float mass = 1.0f;
                std::string physics_type = "dynamic";
                
                if (entity.HasMember("physics") && entity["physics"].IsObject()) {
                    const auto& phys = entity["physics"];
                    mass = phys.HasMember("mass") ? phys["mass"].GetFloat() : 1.0f;
                    physics_type = phys.HasMember("type") ? phys["type"].GetString() : "dynamic";
                }
                
                ecs::njInputComponent input{};
                ecs::njPlayerArchetypeCreateInfo player_info{
                    .name = name,
                    .transform = { .transform = final_transform },
                    .input = input,
                    .mesh = { .mesh = mesh_registry_->get_primary_mesh_name(mesh_alias), .texture_override = "" },
                    .intent = {},
                    .physics = { .mass = mass, .type = physics_type == "dynamic" ? ecs::RigidBodyType::Dynamic : ecs::RigidBodyType::Static }
                };
                ecs::njPlayerArchetype player_archetype{ player_info };
                engine.add_archetype(player_archetype);
                std::cout << "Player '" << name << "' loaded." << std::endl;
                
            } else {
                // Default: object archetype (load all meshes for alias)
                auto mesh_names = mesh_registry_->get_all_mesh_names(mesh_alias);
                std::cout << "Found " << mesh_names.size() << " meshes for '" << mesh_alias << "' alias." << std::endl;
                
                for (const auto& mesh_name : mesh_names) {
                    ecs::njObjectArchetypeCreateInfo object_info{
                        .name = mesh_name,
                        .transform = { .transform = final_transform },
                        .mesh = { .mesh = mesh_name, .texture_override = "" }
                    };
                    ecs::njObjectArchetype object_archetype{ object_info, *mesh_registry_ };
                    engine.add_archetype(object_archetype);
                }
            }
        }
    }
    
    std::cout << "Scene loaded successfully." << std::endl;
}

}  // namespace njin::ecs
