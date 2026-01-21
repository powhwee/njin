#include "mnt/RoomBuilder.h"
using namespace njin;

namespace mnt {
    RoomBuilder::RoomBuilder(RoomSize size,
                             Coordinate coordinate,
                             const njin::core::njRegistry<njin::core::njMesh>& mesh_registry) :
        size_{ size },
        coordinate_{ coordinate },
        mesh_registry_{ &mesh_registry } {}

    std::vector<ecs::njObjectArchetype> RoomBuilder::build() const {
        std::vector<ecs::njObjectArchetype> tiles{};

        for (uint32_t i{ 0 }; i < size_; ++i) {
            for (uint32_t j{ 0 }; j < size_; ++j) {
                // translation of tile
                math::njMat4f location{
                    math::njMat4Type::Translation,
                    { coordinate_.x - static_cast<float>(i * 2),
                      coordinate_.y,
                      coordinate_.z + static_cast<float>(j * 2) }
                };
                ecs::njTransformComponent transform{
                    .transform = location
                };

                ecs::njObjectArchetypeCreateInfo info{
                    .name = "",
                    .transform = transform,
                    .mesh = { .mesh = "cube", .texture_override = "statue" }
                };

                tiles.emplace_back(info, *mesh_registry_);
            }
        }

        return tiles;
    }
}  // namespace mnt
