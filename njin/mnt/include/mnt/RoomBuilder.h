#pragma once
#include <vector>

#include "ecs/Components.h"
#include "ecs/njObjectArchetype.h"
#include "math/njVec3.h"

namespace mnt {
    class RoomBuilder {
        public:
        using Coordinate = njin::math::njVec3f;
        using RoomSize = uint32_t;
        /**
         * Constructor
         * @param size Width of room
         * @param coordinate Coordinate of bottom-left of room
         * @param mesh Mesh to use for the room tiles
         */
        RoomBuilder(RoomSize size,
                    Coordinate coordinate,
                    const std::string& mesh);

        /**
         * Build the room
         * @return Array of archetypes that represent tiles in the room
         */
        std::vector<njin::ecs::njObjectArchetype> build() const;

        private:
        RoomSize size_{};
        Coordinate coordinate_{};
        std::string mesh_{};
    };

}  // namespace mnt