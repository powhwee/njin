#pragma once
#include <string>

#include "core/njRegistry.h"
#include "core/njMesh.h"
#include "core/njMaterial.h"
#include "core/njTexture.h"
#include "ecs/njEngine.h"

namespace njin::ecs {

/**
 * Loads scene configuration from JSON and spawns entities in the engine.
 */
class njSceneLoader {
public:
    /**
     * Constructor
     * @param scene_path Path to the scene JSON file
     * @param mesh_registry Registry of loaded meshes
     * @param material_registry Registry of loaded materials
     * @param texture_registry Registry of loaded textures
     */
    njSceneLoader(const std::string& scene_path,
                  const njin::core::njRegistry<njin::core::njMesh>& mesh_registry,
                  const njin::core::njRegistry<njin::core::njMaterial>& material_registry,
                  const njin::core::njRegistry<njin::core::njTexture>& texture_registry);

    /**
     * Load the scene into the engine, creating camera and entities
     * @param engine Engine to add archetypes to
     */
    void load(njEngine& engine);

private:
    std::string scene_path_;
    const njin::core::njRegistry<njin::core::njMesh>* mesh_registry_;
    const njin::core::njRegistry<njin::core::njMaterial>* material_registry_;
    const njin::core::njRegistry<njin::core::njTexture>* texture_registry_;
};

}  // namespace njin::ecs
