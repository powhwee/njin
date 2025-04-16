#pragma once
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/schema.h>

#include "core/njNode.h"
#include "core/njRegistry.h"
#include "core/njScene.h"

namespace njin::core {

    class njSceneReader {
        public:
        /**
         * Constructor
        * @param path Path to scene to load
        * @throw std::runtime_error if scene.schema.json is not a valid json
        * @throw std::runtime_error if scene json given by path is not a valid json
        */
        njSceneReader(const std::string& path,
                      const njRegistry<njCamera>& camera_registry,
                      const njRegistry<njMesh>& mesh_registry);

        njScene get_scene() const;

        private:
        using NodeIndex = uint32_t;

        njScene scene_{};
    };

}  // namespace njin::core