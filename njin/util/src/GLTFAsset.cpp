#include "util/GLTFAsset.h"

#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>
#include <vulkan/vulkan_core.h>

#include "core/njVertex.h"
#include "core/njMaterial.h"
#include "core/njTexture.h"
#include "math/njVec3.h"
#include "util/Accessor.h"
#include "util/stb.h" // For stb_image functions

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#include <windows.h>
#ifdef GetObject
#undef GetObject
#endif
#endif

uint32_t MAGIC{ 0x46'54'6C'67 };
uint32_t VERSION{ 2 };
uint32_t JSON_CHUNK_TYPE{ 0x4E'4F'53'4A };
uint32_t BIN_CHUNK_TYPE{ 0x00'4E'49'42 };

namespace rj = rapidjson;

namespace {
    namespace gltf = njin::gltf;
    namespace math = njin::math;
    namespace core = njin::core;

    // Helper function to extract raw image data from the glTF binary buffer
    // Helper function to extract raw image data from the glTF binary buffer
    std::vector<unsigned char> get_image_data(const gltf::Buffer& buffer,
                                              const std::vector<gltf::BufferView>& buffer_views,
                                              const rj::Value& image_val) {
        auto image_obj = image_val.GetObject();
        if (image_obj.HasMember("bufferView")) {
            int buffer_view_index = image_obj["bufferView"].GetInt();
            const gltf::BufferView& buffer_view = buffer_views[buffer_view_index];
            std::vector<std::byte> raw_data = buffer_view.get();
            std::vector<unsigned char> result(raw_data.size());
            std::memcpy(result.data(), raw_data.data(), raw_data.size());
            return result;
        }
        // Handle URI for external images if needed in the future
        throw std::runtime_error("External image URIs not yet supported. Image must be embedded via bufferView.");
    }

    std::vector<gltf::BufferView>
    process_buffer_views(const gltf::Buffer& buffer,
                         const rj::Document& document) {
        std::vector<gltf::BufferView> result{};
        rj::GenericArray buffer_views{ document["bufferViews"].GetArray() };
        for (auto it{ buffer_views.begin() }; it != buffer_views.end(); ++it) {
            auto buffer_view = it->GetObject();

            gltf::BufferViewInfo info{};
            info.buffer = buffer_view["buffer"].GetInt();
            if (buffer_view.HasMember("byteOffset")) {
                info.byte_offset = buffer_view["byteOffset"].GetInt();
            }
            info.byte_length = buffer_view["byteLength"].GetInt();
            if (buffer_view.HasMember("byteStride")) {
                info.byte_stride = buffer_view["byteStride"].GetInt();
            }
            if (buffer_view.HasMember("target")) {
                info.target = buffer_view["target"].GetInt();
            }
            if (buffer_view.HasMember("name")) {
                info.name = buffer_view["name"].GetString();
            }

            result.emplace_back(buffer, info);
        }

        return result;
    };

    // Helper function to load image pixels using stb_image and create an njTexture
    // Helper function to load image pixels using stb_image and create an njTexture
    core::njTexture load_image_pixels(const std::vector<unsigned char>& image_data, const std::string& name) {
        int width, height, channels;
        unsigned char* pixels = stbi_load_from_memory(image_data.data(), static_cast<int>(image_data.size()), &width, &height, &channels, STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error(std::format("Failed to load image pixels for texture '{}'", name));
        }

        core::njTextureCreateInfo info{};
        info.width = width;
        info.height = height;
        info.channels = core::TextureChannels::RGBA; // Force to 4 channels (RGBA)
        info.data.assign(pixels, pixels + (width * height * 4));
        info.name = name;

        core::njTexture texture{info};

        stbi_image_free(pixels);
        return texture;
    }

    gltf::Type get_type(const std::string& type) {
        using Type = gltf::Type;
        if (type == "SCALAR") {
            return Type::Scalar;
        } else if (type == "VEC2") {
            return Type::Vec2;
        } else if (type == "VEC3") {
            return Type::Vec3;
        } else if (type == "VEC4") {
            return Type::Vec4;
        } else if (type == "MAT2") {
            return Type::Mat2;
        } else if (type == "MAT3") {
            return Type::Mat3;
        } else if (type == "MAT4") {
            return Type::Mat4;
        }
        throw std::runtime_error("Unknown glTF type: " + type);
    }

    gltf::ComponentType get_component_type(int component_type) {
        using ComponentType = gltf::ComponentType;
        if (component_type == static_cast<int>(ComponentType::SignedByte)) {
            return ComponentType::SignedByte;
        } else if (component_type ==
                   static_cast<int>(ComponentType::UnsignedByte)) {
            return ComponentType::UnsignedByte;
        } else if (component_type ==
                   static_cast<int>(ComponentType::SignedShort)) {
            return ComponentType::SignedShort;
        } else if (component_type ==
                   static_cast<int>(ComponentType::UnsignedShort)) {
            return ComponentType::UnsignedShort;
        } else if (component_type ==
                   static_cast<int>(ComponentType::UnsignedInt)) {
            return ComponentType::UnsignedInt;
        } else if (component_type == static_cast<int>(ComponentType::Float)) {
            return ComponentType::Float;
        }
        throw std::runtime_error("Unsupported component type");
    }

            template<bool Const, typename ValueT>

            gltf::Accessor::AccessorCreateInfo

            make_accessor_create_info(const std::vector<gltf::BufferView>& buffer_views,

                                      const rj::GenericObject<Const, ValueT>&

                                      accessor) {

                int buffer_view_index{ accessor["bufferView"].GetInt() };

        

                // component type

                int gltf_component_type{ accessor["componentType"].GetInt() };

                gltf::ComponentType component_type{

                    get_component_type(gltf_component_type)

                };

        

                // type

                std::string gltf_type{ accessor["type"].GetString() };

                gltf::Type type{ get_type(gltf_type) };

        

                // buffer view

                gltf::BufferView buffer_view{ buffer_views[buffer_view_index] };

        

                // byte offset

                uint32_t byte_offset{ 0 };

                if (accessor.HasMember("byteOffset")) {  // no member = default 0

                    byte_offset = accessor["byteOffset"].GetInt() ;

                }

        

                // count

                uint32_t count{ static_cast<uint32_t>(accessor["count"].GetInt()) };

        

                gltf::Accessor::AccessorCreateInfo info{

                    .type = type,

                    .component_type = component_type,

                    .buffer_view = buffer_view,

                    .byte_offset = byte_offset,

                    .count = count

                };

        

                return info;

            }

        

            std::vector<core::njTexture>

            process_images(const gltf::Buffer& buffer,

                           const std::vector<gltf::BufferView>& buffer_views,

                           const rj::Document& document) {

                std::vector<core::njTexture> result{};

                if (!document.HasMember("images")) {

                    return result;

                }

        

                rj::GenericArray images{ document["images"].GetArray() };

                for (auto it{ images.begin() }; it != images.end(); ++it) {

                    auto image_obj = it->GetObject();

                    // Name from glTF or generated

                    std::string name = image_obj.HasMember("name") ? image_obj["name"].GetString() : std::format("image_{}", result.size());

                    std::vector<unsigned char> image_data = get_image_data(buffer, buffer_views, *it);

                    result.emplace_back(load_image_pixels(image_data, name));

                }

                return result;

            }

        

            std::vector<core::njTexture>

            process_textures(const std::vector<core::njTexture>& images, // These are the raw images, after processing

                             const rj::Document& document) {

                std::vector<core::njTexture> result{};

                if (!document.HasMember("textures")) {

                    return result;

                }

        

                rj::GenericArray textures{ document["textures"].GetArray() };

                for (auto it{ textures.begin() }; it != textures.end(); ++it) {

                    auto texture_obj = it->GetObject();

                    int source_image_index = texture_obj["source"].GetInt();

                    // For now, we directly use the image as the texture. Samplers are ignored.

                    result.emplace_back(images[source_image_index]);

                    result.back().name = texture_obj.HasMember("name") ? texture_obj["name"].GetString() : std::format("texture_{}", result.size());

                }

                return result;

            }

        

            // Temporary struct to hold material info with texture index before resolving to name
            struct TempMaterial {
                core::njMaterial material;
                int base_color_texture_index{ -1 };
            };

            std::vector<TempMaterial>

            process_materials(const std::vector<core::njTexture>& textures,

                              const rj::Document& document) {

                std::vector<TempMaterial> result{};

                if (!document.HasMember("materials")) {

                    return result;

                }

        

                rj::GenericArray materials{ document["materials"].GetArray() };

                for (auto it{ materials.begin() }; it != materials.end(); ++it) {

                    auto material_obj = it->GetObject();

                    TempMaterial temp{};

                    temp.material.name = material_obj.HasMember("name") ? material_obj["name"].GetString() : std::format("material_{}", result.size());

        

                    if (material_obj.HasMember("pbrMetallicRoughness")) {

                        auto pbr = material_obj["pbrMetallicRoughness"].GetObject();

                        if (pbr.HasMember("baseColorFactor")) {

                            const rj::GenericArray color_array = pbr["baseColorFactor"].GetArray();

                            temp.material.base_color_factor = {

                                color_array[0].GetFloat(),

                                color_array[1].GetFloat(),

                                color_array[2].GetFloat(),

                                color_array[3].GetFloat()

                            };

                        }

                        if (pbr.HasMember("baseColorTexture")) {

                            int texture_index = pbr["baseColorTexture"].GetObject()["index"].GetInt();

                            temp.base_color_texture_index = texture_index;
                            printf("[GLTFAsset] Found baseColorTexture index %d for material '%s'\n", texture_index, temp.material.name.c_str());

                        } else {
                            printf("[GLTFAsset] Material '%s' has pbrMetallicRoughness but NO baseColorTexture\n", temp.material.name.c_str());
                        }

                    } else {
                        printf("[GLTFAsset] Material '%s' has NO pbrMetallicRoughness\n", temp.material.name.c_str());
                    }

                    result.emplace_back(temp);

                }

                return result;

            }

        

        }  // namespace

        

        namespace njin::gltf {

        

            GLTFAsset::GLTFAsset(const std::string& path, const std::string& alias)
                : alias_{ alias } {

                std::ifstream file{ path, std::ios::in | std::ios::binary };

                if (!file.is_open()) {

        

    
            throw std::runtime_error("Could not open glTF file!");
        }

        uint32_t bytes;

        // validate the magic number
        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != MAGIC) {
            throw std::runtime_error("File is not a glTF file");
        }

        // check the glTF version (must be 2.0)
        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != VERSION) {
            throw std::runtime_error("njin only accepts glTF 2.0");
        }

        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        length_ = bytes;

        /* Process the json chunk */
        uint32_t json_chunk_length{ 0 };
        file.read(reinterpret_cast<char*>(&json_chunk_length),
                  sizeof(uint32_t));

        // check the chunk type
        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != JSON_CHUNK_TYPE) {
            throw std::runtime_error("first chunk of glb was not a JSON chunk");
        }

        std::string json_chunk;
        json_chunk.resize(json_chunk_length);
        file.read(json_chunk.data(), json_chunk_length);

        rj::Document document;
        document.Parse(json_chunk.c_str());

        /* Process the binary chunk */
        uint32_t binary_chunk_size{ 0 };
        file.read(reinterpret_cast<char*>(&binary_chunk_size),
                  sizeof(uint32_t));

        // check the chunk type
        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != BIN_CHUNK_TYPE) {
            throw std::runtime_error(
            "second chunk of glb was not a binary chunk");
        }

        buffer_ = { file, file.tellg(), binary_chunk_size };

        buffer_views_ = process_buffer_views(buffer_, document);

        rj::GenericArray accessors{ document["accessors"].GetArray() };
        rj::GenericArray meshes{ document["meshes"].GetArray() };

        // Process images, textures, and materials
        textures_ = process_images(buffer_, buffer_views_, document);
        textures_ = process_textures(textures_, document); // Re-process to link images to glTF textures
        auto temp_materials = process_materials(textures_, document);

        // Prefix all texture names with alias
        for (auto& texture : textures_) {
            texture.name = alias_ + "-" + texture.name;
        }

        // Convert temp materials to final materials with prefixed names and resolved texture names
        for (auto& temp : temp_materials) {
            temp.material.name = alias_ + "-" + temp.material.name;
            if (temp.base_color_texture_index >= 0 && temp.base_color_texture_index < static_cast<int>(textures_.size())) {
                temp.material.base_color_texture_name = textures_[temp.base_color_texture_index].name;
                printf("[GLTFAsset] Material '%s' -> texture '%s' (index %d)\n", 
                       temp.material.name.c_str(), 
                       temp.material.base_color_texture_name.c_str(),
                       temp.base_color_texture_index);
            } else {
                printf("[GLTFAsset] Material '%s' has NO texture (index %d, textures count %zu)\n", 
                       temp.material.name.c_str(), 
                       temp.base_color_texture_index,
                       textures_.size());
            }
            materials_.emplace_back(temp.material);
        }

        for (const auto& mesh : meshes) {
            std::string mesh_name = mesh["name"].GetString();
            printf("Loading mesh: %s\n", mesh_name.c_str());
            std::vector<core::njPrimitive> primitives{};

            int primitive_index = 0;
            for (const auto& primitive : mesh["primitives"].GetArray()) {
                printf("  Processing primitive %d\n", primitive_index++);
                // Resolve material index to prefixed material name
                std::string material_name;
                if (primitive.HasMember("material")) {
                    int material_idx = primitive["material"].GetInt();
                    if (material_idx >= 0 && material_idx < static_cast<int>(materials_.size())) {
                        material_name = materials_[material_idx].name;  // Already prefixed
                    }
                }

                // indices
                int indices_accessor_index{ primitive["indices"].GetInt() };
                auto gltf_indices_accessor = accessors[indices_accessor_index].GetObject();
                gltf::Accessor::AccessorCreateInfo indices_accessor_info{
                    make_accessor_create_info(buffer_views_, gltf_indices_accessor)
                };
                gltf::Accessor indices_accessor{ indices_accessor_info };
                std::vector<uint32_t> indices;
                if (indices_accessor_info.component_type == gltf::ComponentType::UnsignedInt) {
                    indices = indices_accessor.get_scalar_u32();
                } else if (indices_accessor_info.component_type == gltf::ComponentType::UnsignedShort) {
                    std::vector<uint16_t> short_indices = indices_accessor.get_scalar();
                    indices.assign(short_indices.begin(), short_indices.end());
                }

                // attributes
                auto attribute = primitive["attributes"].GetObject();

                // position
                int position_accessor_index{ attribute["POSITION"].GetInt() };
                auto gltf_position_accessor = accessors[position_accessor_index].GetObject();
                gltf::Accessor::AccessorCreateInfo position_accessor_info{
                    make_accessor_create_info(buffer_views_, gltf_position_accessor)
                };
                gltf::Accessor position_accessor{ position_accessor_info };
                std::vector<math::njVec3f> positions = position_accessor.get_vec3f();

                // normal
                std::vector<math::njVec3f> normals{};
                if (attribute.HasMember("NORMAL")) {
                    int normal_accessor_index{ attribute["NORMAL"].GetInt() };
                    auto gltf_normal_accessor = accessors[normal_accessor_index].GetObject();
                    gltf::Accessor::AccessorCreateInfo normal_accessor_info{
                        make_accessor_create_info(buffer_views_, gltf_normal_accessor)
                    };
                    gltf::Accessor normal_accessor{ normal_accessor_info };
                    normals = normal_accessor.get_vec3f();
                }

                // tangent
                std::vector<math::njVec4f> tangents{};
                if (attribute.HasMember("TANGENT")) {
                    int tangent_accessor_index{ attribute["TANGENT"].GetInt() };
                    auto gltf_tangent_accessor = accessors[tangent_accessor_index].GetObject();
                    gltf::Accessor::AccessorCreateInfo tangent_accessor_info{
                        make_accessor_create_info(buffer_views_, gltf_tangent_accessor)
                    };
                    gltf::Accessor tangent_accessor{ tangent_accessor_info };
                    tangents = tangent_accessor.get_vec4f();
                }

                // texture coordinates
                std::vector<math::njVec2f> tex_coords{};
                if (attribute.HasMember("TEXCOORD_0")) {
                    int tex_coord_accessor_index{ attribute["TEXCOORD_0"].GetInt() };
                    auto gltf_tex_coord_accessor = accessors[tex_coord_accessor_index].GetObject();
                    Accessor::AccessorCreateInfo tex_coord_accessor_info{
                        make_accessor_create_info(buffer_views_, gltf_tex_coord_accessor)
                    };
                    Accessor tex_coord_accessor{ tex_coord_accessor_info };
                    tex_coords = tex_coord_accessor.get_vec2f();
                }

                // colors
                std::vector<math::njVec4<uint16_t>> colors{};
                if (attribute.HasMember("COLOR_0")) {
                    int color_accessor_index{ attribute["COLOR_0"].GetInt() };
                    auto gltf_color_accessor = accessors[color_accessor_index].GetObject();
                    Accessor::AccessorCreateInfo color_accessor_info{
                        make_accessor_create_info(buffer_views_, gltf_color_accessor)
                    };
                    Accessor color_accessor{ color_accessor_info };
                    colors = color_accessor.get_vec4ushort();
                }

                std::vector<core::njVertex> vertices{};
                for (int i = 0; i < positions.size(); ++i) {
                    core::njVertexCreateInfo create_info{};
                    create_info.position = positions[i];
                    if (!normals.empty()) {
                        create_info.normal = normals[i];
                    }
                    if (!tangents.empty()) {
                        create_info.tangent = tangents[i];
                    }
                    if (!tex_coords.empty()) {
                        create_info.tex_coord = tex_coords[i];
                    }
                    if (!colors.empty()) {
                        create_info.color = colors[i];
                    }
                    vertices.emplace_back(create_info);
                }

                std::fprintf(stderr, "    Primitive has %zu vertices and %zu indices\n", vertices.size(), indices.size());
                primitives.emplace_back(vertices, indices, material_name);
            }

            std::fprintf(stderr, "  Finished loading mesh '%s' with %zu primitives\n", mesh_name.c_str(), primitives.size());
            meshes_.emplace_back(mesh_name, primitives);
        }
    }

    std::vector<core::njMesh> GLTFAsset::get_meshes() const {
        return meshes_;
    }

    std::vector<core::njMaterial> GLTFAsset::get_materials() const {
        return materials_;
    }

    std::vector<core::njTexture> GLTFAsset::get_textures() const {
        return textures_;
    }

}  // namespace njin::gltf