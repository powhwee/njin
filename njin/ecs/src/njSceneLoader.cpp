#include "ecs/njSceneLoader.h"

#include <iostream>

#include <rapidjson/document.h>

#include "ecs/njCameraArchetype.h"
#include "ecs/njObjectArchetype.h"
#include "ecs/njPlayerArchetype.h"
#include "math/njMat4.h"
#include "math/njVec3.h"
#include "math/njVec4.h"
#include "util/json.h"

#include <cmath>

namespace rj = rapidjson;

namespace njin::ecs {

    njSceneLoader::njSceneLoader(
    const std::string& scene_path,
    const njin::core::njRegistry<njin::core::njMesh>& mesh_registry,
    const njin::core::njRegistry<njin::core::njMaterial>& material_registry,
    const njin::core::njRegistry<njin::core::njTexture>& texture_registry,
    const njin::core::njRegistry<njin::core::njSkeleton>& skeleton_registry,
    const njin::core::njRegistry<std::vector<njin::core::njAnimation>>&
    animation_registry) :
        scene_path_{ scene_path },
        mesh_registry_{ &mesh_registry },
        material_registry_{ &material_registry },
        texture_registry_{ &texture_registry },
        skeleton_registry_{ &skeleton_registry },
        animation_registry_{ &animation_registry } {}

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
            float near_plane = cam.HasMember("near") ? cam["near"].GetFloat() :
                                                       0.1f;
            float far_plane = cam.HasMember("far") ? cam["far"].GetFloat() :
                                                     1000.f;
            float aspect = cam.HasMember("aspect") ? cam["aspect"].GetFloat() :
                                                     16.f / 9.f;

            std::string name = cam.HasMember("name") ? cam["name"].GetString() :
                                                       "camera";

            // Parse camera type (default: perspective)
            std::string type_str = cam.HasMember("type") ?
                                   cam["type"].GetString() :
                                   "perspective";
            ecs::njCameraType camera_type;
            if (type_str == "orthographic") {
                camera_type = ecs::njCameraType::Orthographic;
            } else if (type_str == "isometric") {
                camera_type = ecs::njCameraType::Isometric;
            } else {
                camera_type = ecs::njCameraType::Perspective;
            }

            // Create appropriate settings based on type
            // Note: Isometric uses orthographic projection settings
            std::variant<ecs::PerspectiveCameraSettings,
                         ecs::OrthographicCameraSettings>
            settings;
            if (camera_type == ecs::njCameraType::Orthographic ||
                camera_type == ecs::njCameraType::Isometric) {
                float scale = cam.HasMember("scale") ? cam["scale"].GetFloat() :
                                                       10.0f;
                settings = ecs::OrthographicCameraSettings{ .near = near_plane,
                                                            .far = far_plane,
                                                            .scale = scale };
                std::cout << "Camera type: " << type_str << " (scale=" << scale
                          << ")" << std::endl;
            } else {
                settings = ecs::PerspectiveCameraSettings{
                    .near = near_plane,
                    .far = far_plane,
                    .horizontal_fov = fov
                };
                std::cout << "Camera type: perspective (fov=" << fov << ")"
                          << std::endl;
            }

            ecs::njCameraArchetypeCreateInfo camera_info{
                .name = name,
                .transform = ecs::njTransformComponent::make(pos_x,
                                                             pos_y,
                                                             pos_z),
                .camera = { .type = camera_type,
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
                std::string name = entity.HasMember("name") ?
                                   entity["name"].GetString() :
                                   "";
                std::string mesh_alias = entity.HasMember("mesh_alias") ?
                                         entity["mesh_alias"].GetString() :
                                         "";
                std::string archetype_type = entity.HasMember("archetype") ?
                                             entity["archetype"].GetString() :
                                             "object";

                // Get position
                float pos_x = entity.HasMember("position") ?
                              entity["position"][0].GetFloat() :
                              0.f;
                float pos_y = entity.HasMember("position") ?
                              entity["position"][1].GetFloat() :
                              0.f;
                float pos_z = entity.HasMember("position") ?
                              entity["position"][2].GetFloat() :
                              0.f;

                // Build transform with optional rotation
                math::njMat4f translation{
                    math::njMat4Type::Translation,
                    math::njVec3f{ pos_x, pos_y, pos_z }
                };
                math::njMat4f rotation = math::njMat4f::Identity();
                math::njMat4f scale = math::njMat4f::Identity();

                if (entity.HasMember("rotation_x_degrees")) {
                    float angle_x = entity["rotation_x_degrees"].GetFloat() *
                                    3.14159f / 180.f;
                    float half_x = angle_x / 2.f;
                    math::njVec4f quat_x{ std::sin(half_x),
                                          0.f,
                                          0.f,
                                          std::cos(half_x) };
                    rotation = math::njMat4f{ quat_x };
                }

                // Parse optional scale (uniform or per-axis)
                if (entity.HasMember("scale")) {
                    float scale_x = 1.f, scale_y = 1.f, scale_z = 1.f;
                    if (entity["scale"].IsArray()) {
                        scale_x = entity["scale"][0].GetFloat();
                        scale_y = entity["scale"][1].GetFloat();
                        scale_z = entity["scale"][2].GetFloat();
                    } else if (entity["scale"].IsNumber()) {
                        scale_x = scale_y = scale_z =
                        entity["scale"].GetFloat();
                    }
                    scale = math::njMat4f{
                        math::njMat4Type::Scale,
                        math::njVec3f{ scale_x, scale_y, scale_z }
                    };
                }

                // Final global transform of the ROOT
                math::njMat4f final_transform = translation * rotation * scale;

                // CHECK FOR SKELETON
                // We use mesh_alias to find the skeleton.
                const core::njSkeleton* skeleton = nullptr;
                try {
                    skeleton = skeleton_registry_->get(mesh_alias);
                } catch (...) {}

                if (archetype_type == "player" && skeleton) {
                    std::cout << "Loading Animated Player: " << name
                              << " with skeleton from alias " << mesh_alias
                              << std::endl;

                    // Root entity is a container for physics/input/animation state.
                    // The actual mesh is rendered by child skeleton node entities
                    // using animated poses. Root must NOT have a mesh to avoid
                    // a static copy masking the animated one.
                    std::string root_mesh_name = "";

                    ecs::njInputComponent input{};

                    // Parse physics
                    float mass = 1.0f;
                    std::string physics_type = "dynamic";
                    if (entity.HasMember("physics") &&
                        entity["physics"].IsObject()) {
                        const auto& phys = entity["physics"];
                        mass = phys.HasMember("mass") ?
                               phys["mass"].GetFloat() :
                               1.0f;
                        physics_type = phys.HasMember("type") ?
                                       phys["type"].GetString() :
                                       "dynamic";
                    }

                    ecs::njPlayerArchetypeCreateInfo player_info{
                        .name = name,
                        .transform = { .transform = final_transform },
                        .input = input,
                        // We intentionally might NOT want a mesh on the root if the root is just a container.
                        // But let's keep it compatible. If the skeleton handles everything, maybe root has no mesh?
                        // If root_mesh_name is empty, this might crash if njMeshComponent expects string.
                        .mesh = { .mesh = root_mesh_name,
                                  .texture_override = "" },
                        .intent = {},
                        .physics = { .mass = mass,
                                     .type = physics_type == "dynamic" ?
                                             ecs::RigidBodyType::Dynamic :
                                             ecs::RigidBodyType::Static }
                    };
                    ecs::njPlayerArchetype player_archetype{ player_info };
                    EntityId player_id = engine.add_archetype(player_archetype);

                    // 2. Attach Animation Components to Root
                    const std::vector<core::njAnimation>* anims = nullptr;
                    try {
                        anims = animation_registry_->get(mesh_alias);
                    } catch (...) {}

                    ecs::njAnimationComponent anim_comp;
                    anim_comp.animations = anims;
                    anim_comp.playing =
                    true;  // Auto-play (single animation, no key bindings yet)
                    if (anims && !anims->empty()) {
                        anim_comp.current_animation = anims->at(0).name;
                        anim_comp.loop = true;
                    }

                    engine.add_component(player_id, anim_comp);

                    // Add Node Component for Root?
                    // The Skeleton has 'root_nodes'.
                    // This "Player" entity is the "Model Root". It corresponds to NO specific node index usually,
                    // OR it corresponds to a wrapper.
                    // Let's assume the Player Entity is the PARENT of the skeleton roots.
                    // So we attach node component to children.
                    // BUT: njAnimationSystem iterates entities with (njNodeComponent, njAnimationComponent).
                    // Wait, my design in Step 171 iterates `get_view<njNodeComponent, njAnimationComponent>`.
                    // This implies the ROOT entity MUST have `njNodeComponent`.
                    // But which node index?
                    // GLTF has "scenes" with "nodes". A scene has root nodes.
                    // The "Model" itself is the scene.
                    //
                    // Correction: The `njAnimationComponent` holds the `pose` for ALL nodes.
                    // It doesn't necessarily need to be a node itself.
                    // But `njAnimationSystem` REQUIRES `njNodeComponent` to get the `skeleton` pointer!
                    // `njNodeComponent` holds `njSkeleton*`.
                    // So yes, we need `njNodeComponent` on the player entity just to point to the Skeleton.
                    // `node_index` can be -1 (ignored) if it's just a container.

                    ecs::njNodeComponent root_node_comp;
                    root_node_comp.skeleton = skeleton;
                    root_node_comp.node_index = -1;
                    engine.add_component(player_id, root_node_comp);

                    // Add Bindings — auto-bind animations to number keys
                    ecs::njAnimationBindingsComponent bindings;
                    if (anims && !anims->empty()) {
                        SDL_Scancode number_keys[] = {
                            SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
                            SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6,
                            SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9
                        };
                        for (size_t i = 0; i < anims->size() && i < 9; ++i) {
                            bindings.key_to_animation[number_keys[i]] =
                            anims->at(i).name;
                            std::cout << "Animation key " << (i + 1) << " -> \""
                                      << anims->at(i).name << "\"" << std::endl;
                        }
                    }
                    engine.add_component(player_id, bindings);

                    // 3. Spawning Children (Runtime Hierarchy)
                    // We traverse the skeleton and spawn entities for each node that has a mesh (or all nodes?)
                    // If we spawn for ALL nodes, we have a full hierarchy.
                    // Let's spawn for ALL nodes to support attaching things to bones later.

                    // Recursive helper
                    // Note: GLTF node transforms are local to parent.
                    // Our system:
                    // Root Entity (Player) -> Global Transform
                    //   -> Skeleton Root Nodes (Child Entities) -> Local Transform relative to Player

                    auto spawn_node =
                    [&](auto self, int node_idx, int parent_entity_id) -> void {
                        const auto& node = skeleton->nodes[node_idx];
                        std::string entity_name = name + "_" + node.name;

                        // Determine mesh — prefix with alias to match registry key format
                        std::string node_mesh_name;
                        if (node.mesh_index >= 0 && !node.mesh_name.empty()) {
                            node_mesh_name = mesh_alias + "-" + node.mesh_name;
                        }

                        // Create Entity
                        ecs::njObjectArchetypeCreateInfo node_info{
                            .name = entity_name,
                            .transform = ecs::njTransformComponent::make(
                            0,
                            0,
                            0),  // Identity initially, driven by AnimSystem
                            .mesh = { .mesh = node_mesh_name,
                                      .texture_override = "" }
                        };

                        // Note: njObjectArchetype requires a valid mesh name in registry?
                        // If node has no mesh, we might fail or need empty archetype.
                        // check if mesh exists
                        bool has_mesh = !node_mesh_name.empty();

                        EntityId ent_id;
                        if (has_mesh) {
                            ecs::njObjectArchetype arch{ node_info,
                                                         *mesh_registry_ };
                            ent_id = engine.add_archetype(arch);
                        } else {
                            // Create empty entity (no mesh) using add_entity
                            std::string entity_name = "skeleton_node_" +
                                                      std::to_string(node_idx);
                            ent_id = engine.add_entity(entity_name);
                            // Add basic components
                            engine
                            .add_component(ent_id,
                                           ecs::njTransformComponent::make(0,
                                                                           0,
                                                                           0));
                            // Add Parent component
                        }

                        // Hierarchy linking
                        engine.add_component(
                        ent_id,
                        ecs::njParentComponent{
                            .id = static_cast<EntityId>(parent_entity_id) });

                        // Render linking (SkeletonRef)
                        engine.add_component(ent_id,
                                             ecs::njSkeletonRefComponent{
                                                 .root_entity = player_id,
                                                 .node_index = node_idx });

                        // Recurse
                        for (int child_idx : node.children) {
                            self(self, child_idx, ent_id);
                        }
                    };

                    for (int root_idx : skeleton->root_nodes) {
                        spawn_node(spawn_node, root_idx, player_id);
                    }

                } else if (archetype_type == "player") {
                    // LEGACY PLAYER PATH (No Skeleton found)
                    float mass = 1.0f;
                    std::string physics_type = "dynamic";

                    if (entity.HasMember("physics") &&
                        entity["physics"].IsObject()) {
                        const auto& phys = entity["physics"];
                        mass = phys.HasMember("mass") ?
                               phys["mass"].GetFloat() :
                               1.0f;
                        physics_type = phys.HasMember("type") ?
                                       phys["type"].GetString() :
                                       "dynamic";
                    }

                    auto mesh_names =
                    mesh_registry_->get_all_mesh_names(mesh_alias);
                    if (mesh_names.empty()) {
                        std::cerr << "Error: No meshes found for alias '"
                                  << mesh_alias << "'" << std::endl;
                        continue;
                    }

                    ecs::njInputComponent input{};
                    ecs::njPlayerArchetypeCreateInfo player_info{
                        .name = name,
                        .transform = { .transform = final_transform },
                        .input = input,
                        .mesh = { .mesh = mesh_names[0],
                                  .texture_override =
                                  "" },  // Use first mesh for root
                        .intent = {},
                        .physics = { .mass = mass,
                                     .type = physics_type == "dynamic" ?
                                             ecs::RigidBodyType::Dynamic :
                                             ecs::RigidBodyType::Static }
                    };
                    ecs::njPlayerArchetype player_archetype{ player_info };
                    EntityId player_id = engine.add_archetype(player_archetype);
                    std::cout << "Player '" << name
                              << "' loaded (Root ID: " << player_id
                              << ") with mesh: " << mesh_names[0] << std::endl;

                    // Create child entities for remaining mesh parts
                    for (size_t i = 1; i < mesh_names.size(); ++i) {
                        ecs::njObjectArchetypeCreateInfo child_info{
                            .name = name + "_part_" + std::to_string(i),
                            .transform = ecs::njTransformComponent::make(
                            0,
                            0,
                            0),  // Identity relative to parent
                            .mesh = { .mesh = mesh_names[i],
                                      .texture_override = "" }
                        };
                        ecs::njObjectArchetype child_archetype{
                            child_info,
                            *mesh_registry_
                        };
                        EntityId child_id =
                        engine.add_archetype(child_archetype);

                        engine.add_component(child_id,
                                             ecs::njParentComponent{
                                                 .id = player_id });
                        std::cout << "  - Loaded child part: " << mesh_names[i]
                                  << " (ID: " << child_id << ")" << std::endl;
                    }

                } else {
                    // Default: object archetype (load all meshes for alias)
                    // TODO: Should we also use skeleton for non-player objects?
                    // For now, keep legacy behavior for static objects.
                    auto mesh_names =
                    mesh_registry_->get_all_mesh_names(mesh_alias);
                    std::cout << "Found " << mesh_names.size()
                              << " meshes for '" << mesh_alias << "' alias."
                              << std::endl;

                    for (const auto& mesh_name : mesh_names) {
                        ecs::njObjectArchetypeCreateInfo object_info{
                            .name = mesh_name,
                            .transform = { .transform = final_transform },
                            .mesh = { .mesh = mesh_name,
                                      .texture_override = "" }
                        };
                        ecs::njObjectArchetype object_archetype{
                            object_info,
                            *mesh_registry_
                        };
                        engine.add_archetype(object_archetype);
                    }
                }
            }
        }
        std::cout << "Scene loaded successfully." << std::endl;
    }

}  // namespace njin::ecs
