#include "core/loader.h"

#include <core/MeshBuilder.h>

#include "util/GLTFAsset.h"
#include "util/json.h"
#include "util/stb.h"

#include <set>

namespace rj = rapidjson;

namespace njin::core {
    void load_meshes(const std::string& path, njRegistry<njMesh>& mesh_registry, njRegistry<njMaterial>& material_registry, njRegistry<njTexture>& texture_registry) {
        // Check that the schema for the config is a valid json
        rj::Document document{
            njin::util::make_validated_document("schema/meshes.schema.json",
                                                path)
        };

        rj::GenericArray meshes_to_load{ document["meshes"].GetArray() };
        for (const auto& gltf_mesh_info : meshes_to_load) {
            std::string name{ gltf_mesh_info["name"].GetString() };
            std::string mesh_path{ gltf_mesh_info["path"].GetString() };
            njin::gltf::GLTFAsset asset{ mesh_path, name };

            for (auto& mesh : asset.get_meshes()) {
                std::string registry_key = name + "-" + mesh.name;
                std::cout << "Registered mesh: " << registry_key << std::endl;
                mesh_registry.add(registry_key, mesh);
            }

            // Materials and textures are already prefixed with alias by GLTFAsset
            for (const auto& material : asset.get_materials()) {
                std::cout << "Registered material: " << material.name << std::endl;
                material_registry.add(material.name, material);
            }

            for (const auto& texture : asset.get_textures()) {
                std::cout << "Registered texture: " << texture.name << std::endl;
                texture_registry.add(texture.name, texture);
            }
        }
    }

    void load_textures(const std::string& path,
                       njRegistry<njTexture>& texture_registry) {
        rj::Document document{
            util::make_validated_document("schema/textures.schema.json", path)
        };

        rj::GenericArray textures{ document["textures"].GetArray() };
        for (const auto& rj_texture : textures) {
            std::string name{ rj_texture["name"].GetString() };
            std::string texture_path{ rj_texture["path"].GetString() };
            int tex_width;
            int tex_height;
            int tex_channels;
            stbi_uc* pixels{ stbi_load(texture_path.c_str(),
                                       &tex_width,
                                       &tex_height,
                                       &tex_channels,
                                       STBI_rgb_alpha) };

            size_t image_size{ static_cast<size_t>(tex_width * tex_height *
                                                   4) };

            if (!pixels) {
                throw std::runtime_error("failed to load texture image");
            }
            std::vector<stbi_uc> data{ pixels, pixels + image_size };
            stbi_image_free(pixels);

            njTextureCreateInfo info{ .data = data,
                                      .width = tex_width,
                                      .height = tex_height,
                                      .channels = TextureChannels::RGBA };
            njTexture texture{ info };
            texture_registry.add(name, texture);
        }
    }
}  // namespace njin::core