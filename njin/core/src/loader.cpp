#include "core/loader.h"

#include <core/MeshBuilder.h>

#include "util/GLTFAsset.h"
#include "util/json.h"
#include "util/stb.h"

#include <set>

namespace rj = rapidjson;

namespace njin::core {
    void load_meshes(const std::string& path,
                     njRegistry<njMesh>& mesh_registry) {
        // Check that the schema for the config is a valid json
        rj::Document document{
            njin::util::make_validated_document("schema/meshes.schema.json",
                                                path)
        };

        rj::GenericArray meshes{ document["meshes"].GetArray() };
        for (const auto& gltf_mesh : meshes) {
            std::string name{ gltf_mesh["name"].GetString() };
            std::string mesh_path{ gltf_mesh["path"].GetString() };
            njin::gltf::GLTFAsset asset{ mesh_path };

            // get the attribute arrays
            auto position_attributes{ asset.get_position_attributes() };
            auto normal_attributes{ asset.get_normal_attributes() };
            auto tangent_attributes{ asset.get_tangent_attributes() };
            auto tex_coords{ asset.get_texture_coordinates() };
            auto color_attributes{ asset.get_color_attributes() };
            auto indices{ asset.get_indices() };

            core::MeshBuilder mesh_builder{ indices };
            mesh_builder.add_position_attributes(position_attributes);
            mesh_builder.add_normal_attributes(normal_attributes);
            mesh_builder.add_tangent_attributes(tangent_attributes);
            mesh_builder.add_texture_coordinates(tex_coords);
            mesh_builder.add_color_attributes(color_attributes);

            core::njMesh mesh{ mesh_builder.build() };
            mesh_registry.add(name, mesh);
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