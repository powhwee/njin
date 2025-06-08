#include "mnt/TownLevel.h"

#include "ecs/njCameraArchetype.h"
#include "mnt/RoomBuilder.h"

namespace mnt {

    TownLevel::TownLevel(njin::ecs::njEngine& engine) : Level{ engine } {}

    void TownLevel::load() {
        // RoomBuilder::RoomSize size{ 5 };
        // RoomBuilder::Coordinate origin{ 0, 0, 0 };
        // RoomBuilder builder{ size, origin, "rocks" };
        // std::vector<njin::ecs::njObjectArchetype> tiles{ builder.build() };
        // for (const njin::ecs::njObjectArchetype& tile_archetype : tiles) {
        //     engine_->add_archetype(tile_archetype);
        // }

        njin::ecs::OrthographicCameraSettings camera_settings{
            .near = { 1.f },
            .far = { 1000.f },
            .scale = { 10 }
        };
        njin::ecs::njCameraArchetypeCreateInfo camera_info{
            .name = "camera",
            .transform = njin::ecs::njTransformComponent::make(-10.f,
                                                               8.f,
                                                               -10.f),
            .camera = { .type = njin::ecs::njCameraType::Orthographic,
                        .up = { 0.f, 1.f, 0.f },
                        .look_at = { 0.f, 0.f, 0.f },
                        .aspect = { 16.f / 9.f },
                        .settings = camera_settings }
        };

        njin::ecs::njCameraArchetype camera_archetype{ camera_info };
        engine_->add_archetype(camera_archetype);

        njin::ecs::njPlayerArchetypeCreateInfo player_archetype_info{
            .name = "player",
            .transform = njin::ecs::njTransformComponent::make(0.f, 1.f, 0.f),
            .input = {},
            .mesh = { .mesh = "player", .texture = "player" },
            .intent = { .velocity = {}, .velocity_override = true },
            .physics = { .velocity = {},
                         .force = {},
                         .mass = 80,
                         .collider = { .transform = { njin::math::njMat4Type::
                                                      Translation,
                                                      { 0.f, 0.f, 0.f } },
                                       .x_width = 1,
                                       .z_width = 1 } }
        };
        njin::ecs::njPlayerArchetype player_archetype{ player_archetype_info };
        engine_->add_archetype(player_archetype);
    }
}  // namespace mnt
