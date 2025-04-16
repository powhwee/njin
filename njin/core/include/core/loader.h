#pragma once
#include "core/njMesh.h"
#include "core/njRegistry.h"
#include "core/njTexture.h"
#include "util/stb.h"

namespace njin::core {

    /**
 * Loads meshes specified in a .meshes file into a given mesh registry
 * @param path Path to .meshes file
 * @param mesh_registry Mesh registry to populate
 */
    void load_meshes(const std::string& path,
                     njRegistry<njMesh>& mesh_registry);

    /**
   * Loads textures specified in a .textures file into a given texture registry
   * @param path Path to .textures file
   * @param texture_registry Texture registry to load textures into
   */
    void load_textures(const std::string& path,
                       njRegistry<njTexture>& texture_registry);

}  // namespace njin::core