#pragma once
#include "core/njMesh.h"
#include "core/njMaterial.h"
#include "core/njRegistry.h"
#include "core/njTexture.h"
#include "util/stb.h"

namespace njin::core {

    /**
     * Loads all assets (meshes, materials, textures) from the glTF files
     * specified in a manifest file.
     * @param path Path to the asset manifest file.
     * @param mesh_registry Registry to populate with loaded meshes.
     * @param material_registry Registry to populate with loaded materials.
     * @param texture_registry Registry to populate with textures from the glTFs.
     */
    void load_meshes(const std::string& path, njRegistry<njMesh>& mesh_registry, njRegistry<njMaterial>& material_registry, njRegistry<njTexture>& texture_registry);

    /**
   * Loads textures specified in a .textures file into a given texture registry
   * @param path Path to .textures file
   * @param texture_registry Texture registry to load textures into
   */
    void load_textures(const std::string& path,
                       njRegistry<njTexture>& texture_registry);

}  // namespace njin::core