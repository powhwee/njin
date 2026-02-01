#include "mnt/RoomBuilder.h"

using namespace njin;

namespace mnt {
    RoomBuilder::RoomBuilder(RoomSize size,
                             Coordinate coordinate,
                             const std::string& mesh_alias,
                             const njin::core::njRegistry<njin::core::njMesh>& mesh_registry) :
        size_{ size },
        coordinate_{ coordinate },
        mesh_alias_{ mesh_alias },
        mesh_registry_{ &mesh_registry } {}

    std::vector<ecs::njObjectArchetype> RoomBuilder::build() const {
        std::vector<ecs::njObjectArchetype> tiles{};

        for (uint32_t i{ 0 }; i < size_; ++i) {
            for (uint32_t j{ 0 }; j < size_; ++j) {
                float x = coordinate_.x + static_cast<float>(i * 5);
                float y = coordinate_.y;
                float z = coordinate_.z - static_cast<float>(j * 5);
                
                // translation of tile
                math::njMat4f location{
                    math::njMat4Type::Translation,
                    { x, y, z }
                };
                ecs::njTransformComponent transform{
                    .transform = location
                };

                ecs::njObjectArchetypeCreateInfo info{
                    .name = "tile_" + std::to_string(i) + "_" + std::to_string(j),
                    .transform = transform,
                    .mesh = { .mesh = mesh_registry_->get_primary_mesh_name(mesh_alias_), .texture_override = "" }
                };

                tiles.emplace_back(info, *mesh_registry_);
            }
        }

        return tiles;
    }
}  // namespace mnt
