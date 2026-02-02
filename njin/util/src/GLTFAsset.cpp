#include "util/GLTFAsset.h"

#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>
#include <vulkan/vulkan_core.h>

#include "core/njMaterial.h"
#include "core/njTexture.h"
#include "core/njVertex.h"
#include "math/njMat4.h"
#include "math/njVec3.h"
#include "util/Accessor.h"
#include "util/stb.h"  // For stb_image functions

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
    std::vector<unsigned char>
    get_image_data(const gltf::Buffer& buffer,
                   const std::vector<gltf::BufferView>& buffer_views,
                   const rj::Value& image_val) {
        auto image_obj = image_val.GetObject();
        if (image_obj.HasMember("bufferView")) {
            int buffer_view_index = image_obj["bufferView"].GetInt();
            const gltf::BufferView& buffer_view =
            buffer_views[buffer_view_index];
            std::vector<std::byte> raw_data = buffer_view.get();
            std::vector<unsigned char> result(raw_data.size());
            std::memcpy(result.data(), raw_data.data(), raw_data.size());
            return result;
        }
        throw std::runtime_error("External image URIs not yet supported. Image "
                                 "must be embedded via bufferView.");
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

    core::njTexture
    load_image_pixels(const std::vector<unsigned char>& image_data,
                      const std::string& name) {
        int width, height, channels;
        unsigned char* pixels =
        stbi_load_from_memory(image_data.data(),
                              static_cast<int>(image_data.size()),
                              &width,
                              &height,
                              &channels,
                              STBI_rgb_alpha);

        if (!pixels) {
            throw std::runtime_error(
            std::format("Failed to load image pixels for texture '{}'", name));
        }

        core::njTextureCreateInfo info{};
        info.width = width;
        info.height = height;
        info.channels = core::TextureChannels::RGBA;
        info.data.assign(pixels, pixels + (width * height * 4));
        info.name = name;

        core::njTexture texture{ info };

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
        int gltf_component_type{ accessor["componentType"].GetInt() };
        gltf::ComponentType component_type{
            get_component_type(gltf_component_type)
        };
        std::string gltf_type{ accessor["type"].GetString() };
        gltf::Type type{ get_type(gltf_type) };
        gltf::BufferView buffer_view{ buffer_views[buffer_view_index] };
        uint32_t byte_offset{ 0 };
        if (accessor.HasMember("byteOffset")) {
            byte_offset = accessor["byteOffset"].GetInt();
        }
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
            std::string name = image_obj.HasMember("name") ?
                               image_obj["name"].GetString() :
                               std::format("image_{}", result.size());
            std::vector<unsigned char> image_data = get_image_data(buffer,
                                                                   buffer_views,
                                                                   *it);
            result.emplace_back(load_image_pixels(image_data, name));
        }
        return result;
    }

    std::vector<core::njTexture>
    process_textures(const std::vector<core::njTexture>& images,
                     const rj::Document& document) {
        std::vector<core::njTexture> result{};
        if (!document.HasMember("textures")) {
            return result;
        }
        rj::GenericArray textures{ document["textures"].GetArray() };
        for (auto it{ textures.begin() }; it != textures.end(); ++it) {
            auto texture_obj = it->GetObject();
            int source_image_index = texture_obj["source"].GetInt();
            result.emplace_back(images[source_image_index]);
            result.back().name = texture_obj.HasMember("name") ?
                                 texture_obj["name"].GetString() :
                                 std::format("texture_{}", result.size());
        }
        return result;
    }

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
            temp.material.name = material_obj.HasMember("name") ?
                                 material_obj["name"].GetString() :
                                 std::format("material_{}", result.size());

            if (material_obj.HasMember("pbrMetallicRoughness")) {
                auto pbr = material_obj["pbrMetallicRoughness"].GetObject();
                if (pbr.HasMember("baseColorFactor")) {
                    const rj::GenericArray color_array =
                    pbr["baseColorFactor"].GetArray();
                    temp.material.base_color_factor = {
                        color_array[0].GetFloat(),
                        color_array[1].GetFloat(),
                        color_array[2].GetFloat(),
                        color_array[3].GetFloat()
                    };
                }
                if (pbr.HasMember("baseColorTexture")) {
                    int texture_index =
                    pbr["baseColorTexture"].GetObject()["index"].GetInt();
                    temp.base_color_texture_index = texture_index;
                }
            }
            result.emplace_back(temp);
        }
        return result;
    }
}  // namespace

namespace njin::gltf {

    // Internal structs to hold raw data before transform baking
    struct RawPrimitive {
        std::vector<core::njVertex> vertices;
        std::vector<uint32_t> indices;
        std::string material_name;
    };

    struct RawMesh {
        std::string name;
        std::vector<RawPrimitive> primitives;
    };

    GLTFAsset::GLTFAsset(const std::string& path, const std::string& alias) :
        alias_{ alias } {
        std::ifstream file{ path, std::ios::in | std::ios::binary };

        if (!file.is_open()) {
            throw std::runtime_error("Could not open glTF file!");
        }

        uint32_t bytes;
        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != MAGIC)
            throw std::runtime_error("File is not a glTF file");

        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != VERSION)
            throw std::runtime_error("njin only accepts glTF 2.0");

        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        length_ = bytes;

        uint32_t json_chunk_length{ 0 };
        file.read(reinterpret_cast<char*>(&json_chunk_length),
                  sizeof(uint32_t));

        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != JSON_CHUNK_TYPE)
            throw std::runtime_error("first chunk of glb was not a JSON chunk");

        std::string json_chunk;
        json_chunk.resize(json_chunk_length);
        file.read(json_chunk.data(), json_chunk_length);

        rj::Document document;
        document.Parse(json_chunk.c_str());

        uint32_t binary_chunk_size{ 0 };
        file.read(reinterpret_cast<char*>(&binary_chunk_size),
                  sizeof(uint32_t));

        file.read(reinterpret_cast<char*>(&bytes), sizeof(uint32_t));
        if (bytes != BIN_CHUNK_TYPE)
            throw std::runtime_error(
            "second chunk of glb was not a binary chunk");

        buffer_ = { file, file.tellg(), binary_chunk_size };
        buffer_views_ = process_buffer_views(buffer_, document);

        rj::GenericArray accessors{ document["accessors"].GetArray() };
        rj::GenericArray meshes{ document["meshes"].GetArray() };

        textures_ = process_images(buffer_, buffer_views_, document);
        textures_ = process_textures(textures_, document);
        auto temp_materials = process_materials(textures_, document);

        for (auto& texture : textures_) {
            texture.name = alias_ + "-" + texture.name;
        }

        for (auto& temp : temp_materials) {
            temp.material.name = alias_ + "-" + temp.material.name;
            if (temp.base_color_texture_index >= 0 &&
                temp.base_color_texture_index <
                static_cast<int>(textures_.size())) {
                temp.material.base_color_texture_name =
                textures_[temp.base_color_texture_index].name;
            }
            materials_.emplace_back(temp.material);
        }

        // 1. Load RAW meshes into temporary structure
        std::vector<RawMesh> raw_meshes;
        for (const auto& mesh : meshes) {
            std::string mesh_name = mesh["name"].GetString();
            std::vector<RawPrimitive> primitives{};

            for (const auto& primitive : mesh["primitives"].GetArray()) {
                std::string material_name;
                if (primitive.HasMember("material")) {
                    int material_idx = primitive["material"].GetInt();
                    if (material_idx >= 0 &&
                        material_idx < static_cast<int>(materials_.size())) {
                        material_name = materials_[material_idx].name;
                    }
                }

                int indices_accessor_index{ primitive["indices"].GetInt() };
                auto gltf_indices_accessor =
                accessors[indices_accessor_index].GetObject();
                gltf::Accessor::AccessorCreateInfo indices_accessor_info{
                    make_accessor_create_info(buffer_views_,
                                              gltf_indices_accessor)
                };
                gltf::Accessor indices_accessor{ indices_accessor_info };
                std::vector<uint32_t> indices;
                if (indices_accessor_info.component_type ==
                    gltf::ComponentType::UnsignedInt) {
                    indices = indices_accessor.get_scalar_u32();
                } else if (indices_accessor_info.component_type ==
                           gltf::ComponentType::UnsignedShort) {
                    std::vector<uint16_t> short_indices =
                    indices_accessor.get_scalar();
                    indices.assign(short_indices.begin(), short_indices.end());
                }

                auto attribute = primitive["attributes"].GetObject();
                int position_accessor_index{ attribute["POSITION"].GetInt() };
                auto gltf_position_accessor =
                accessors[position_accessor_index].GetObject();
                gltf::Accessor::AccessorCreateInfo position_accessor_info{
                    make_accessor_create_info(buffer_views_,
                                              gltf_position_accessor)
                };
                gltf::Accessor position_accessor{ position_accessor_info };
                std::vector<math::njVec3f> positions =
                position_accessor.get_vec3f();

                std::vector<math::njVec3f> normals{};
                if (attribute.HasMember("NORMAL")) {
                    int normal_accessor_index{ attribute["NORMAL"].GetInt() };
                    auto gltf_normal_accessor =
                    accessors[normal_accessor_index].GetObject();
                    gltf::Accessor::AccessorCreateInfo normal_accessor_info{
                        make_accessor_create_info(buffer_views_,
                                                  gltf_normal_accessor)
                    };
                    gltf::Accessor normal_accessor{ normal_accessor_info };
                    normals = normal_accessor.get_vec3f();
                }

                std::vector<math::njVec4f> tangents{};
                if (attribute.HasMember("TANGENT")) {
                    int tangent_accessor_index{ attribute["TANGENT"].GetInt() };
                    auto gltf_tangent_accessor =
                    accessors[tangent_accessor_index].GetObject();
                    gltf::Accessor::AccessorCreateInfo tangent_accessor_info{
                        make_accessor_create_info(buffer_views_,
                                                  gltf_tangent_accessor)
                    };
                    gltf::Accessor tangent_accessor{ tangent_accessor_info };
                    tangents = tangent_accessor.get_vec4f();
                }

                std::vector<math::njVec2f> tex_coords{};
                if (attribute.HasMember("TEXCOORD_0")) {
                    int tex_coord_accessor_index{
                        attribute["TEXCOORD_0"].GetInt()
                    };
                    auto gltf_tex_coord_accessor =
                    accessors[tex_coord_accessor_index].GetObject();
                    Accessor::AccessorCreateInfo tex_coord_accessor_info{
                        make_accessor_create_info(buffer_views_,
                                                  gltf_tex_coord_accessor)
                    };
                    Accessor tex_coord_accessor{ tex_coord_accessor_info };
                    tex_coords = tex_coord_accessor.get_vec2f();
                }

                std::vector<math::njVec4<uint16_t>> colors{};
                if (attribute.HasMember("COLOR_0")) {
                    int color_accessor_index{ attribute["COLOR_0"].GetInt() };
                    auto gltf_color_accessor =
                    accessors[color_accessor_index].GetObject();
                    Accessor::AccessorCreateInfo color_accessor_info{
                        make_accessor_create_info(buffer_views_,
                                                  gltf_color_accessor)
                    };
                    Accessor color_accessor{ color_accessor_info };
                    colors = color_accessor.get_vec4ushort();
                }

                std::vector<core::njVertex> vertices{};
                for (int i = 0; i < positions.size(); ++i) {
                    core::njVertexCreateInfo create_info{};
                    create_info.position = positions[i];
                    if (!normals.empty())
                        create_info.normal = normals[i];
                    if (!tangents.empty())
                        create_info.tangent = tangents[i];
                    if (!tex_coords.empty())
                        create_info.tex_coord = tex_coords[i];
                    if (!colors.empty())
                        create_info.color = colors[i];
                    vertices.emplace_back(create_info);
                }
                primitives.emplace_back(vertices, indices, material_name);
            }
            raw_meshes.push_back({ mesh_name, primitives });
        }

        // 2. Process Nodes
        std::vector<Node> nodes;
        if (document.HasMember("nodes")) {
            const auto& nodes_array = document["nodes"].GetArray();
            int node_idx = 0;
            for (const auto& node_val : nodes_array) {
                Node node;
                if (node_val.HasMember("name"))
                    node.name = node_val["name"].GetString();
                else
                    node.name = "node_" + std::to_string(node_idx);

                if (node_val.HasMember("mesh"))
                    node.mesh_index = node_val["mesh"].GetInt();

                if (node_val.HasMember("children")) {
                    for (const auto& child : node_val["children"].GetArray()) {
                        node.children.push_back(child.GetInt());
                    }
                }

                math::njVec3f translation = { 0, 0, 0 };
                math::njVec3f scale = { 1, 1, 1 };
                math::njVec4f rotation = { 0, 0, 0, 1 };

                if (node_val.HasMember("matrix")) {
                    const auto& matrix = node_val["matrix"].GetArray();
                    // GLTF is Column-Major, but njMat4 constructor takes Rows.
                    // We need to transpose: Col0 -> Row0, etc.
                    math::njVec4f r0{ matrix[0].GetFloat(),
                                      matrix[4].GetFloat(),
                                      matrix[8].GetFloat(),
                                      matrix[12].GetFloat() };
                    math::njVec4f r1{ matrix[1].GetFloat(),
                                      matrix[5].GetFloat(),
                                      matrix[9].GetFloat(),
                                      matrix[13].GetFloat() };
                    math::njVec4f r2{ matrix[2].GetFloat(),
                                      matrix[6].GetFloat(),
                                      matrix[10].GetFloat(),
                                      matrix[14].GetFloat() };
                    math::njVec4f r3{ matrix[3].GetFloat(),
                                      matrix[7].GetFloat(),
                                      matrix[11].GetFloat(),
                                      matrix[15].GetFloat() };
                    node.transform = math::njMat4f(r0, r1, r2, r3);
                } else {
                    if (node_val.HasMember("translation")) {
                        const auto& t = node_val["translation"].GetArray();
                        translation = { t[0].GetFloat(),
                                        t[1].GetFloat(),
                                        t[2].GetFloat() };
                    }
                    if (node_val.HasMember("rotation")) {
                        const auto& r = node_val["rotation"].GetArray();
                        rotation = { r[0].GetFloat(),
                                     r[1].GetFloat(),
                                     r[2].GetFloat(),
                                     r[3].GetFloat() };
                    }
                    if (node_val.HasMember("scale")) {
                        const auto& s = node_val["scale"].GetArray();
                        scale = { s[0].GetFloat(),
                                  s[1].GetFloat(),
                                  s[2].GetFloat() };
                    }

                    math::njMat4f t_mat{ math::njMat4Type::Translation,
                                         translation };
                    math::njMat4f r_mat{ rotation };
                    math::njMat4f s_mat{ math::njMat4Type::Scale, scale };
                    node.transform = t_mat * r_mat * s_mat;
                }

                nodes.push_back(node);
                node_idx++;
            }
        }

        // 3. Bake Transforms via Hierarchy Traversal
        std::vector<int> root_nodes;
        if (document.HasMember("scenes")) {
            int scene_index = document.HasMember("scene") ?
                              document["scene"].GetInt() :
                              0;
            const auto& scenes = document["scenes"].GetArray();
            if (scene_index < scenes.Size()) {
                const auto& scene = scenes[scene_index];
                if (scene.HasMember("nodes")) {
                    for (const auto& node_idx : scene["nodes"].GetArray()) {
                        root_nodes.push_back(node_idx.GetInt());
                    }
                }
            }
        }

        // Helper to recursively process
        auto process_node = [&](auto self,
                                int node_index,
                                const math::njMat4f& parent_transform) -> void {
            if (node_index >= nodes.size())
                return;
            const Node& node = nodes[node_index];
            math::njMat4f global_transform = parent_transform * node.transform;

            if (node.mesh_index >= 0 && node.mesh_index < raw_meshes.size()) {
                const RawMesh& raw_mesh = raw_meshes[node.mesh_index];
                // Appending Node name creates a unique registry key for this specific instance/transform
                std::string processed_name = raw_mesh.name + "_" + node.name;

                std::vector<core::njPrimitive> processed_primitives;
                for (const auto& prim : raw_mesh.primitives) {
                    std::vector<core::njVertex> transformed_vertices =
                    prim.vertices;
                    for (auto& v : transformed_vertices) {
                        // Position (w=1)
                        math::njVec4f pos4 = { v.position.x,
                                               v.position.y,
                                               v.position.z,
                                               1.0f };
                        pos4 = global_transform * pos4;
                        v.position = { pos4.x, pos4.y, pos4.z };

                        // Normal
                        math::njVec4f norm4 = { v.normal.x,
                                                v.normal.y,
                                                v.normal.z,
                                                0.0f };
                        norm4 = global_transform * norm4;
                        math::njVec3f new_norm = { norm4.x, norm4.y, norm4.z };
                        v.normal = math::normalize(new_norm);

                        // Tangent
                        math::njVec4f tan4 = { v.tangent.x,
                                               v.tangent.y,
                                               v.tangent.z,
                                               0.0f };
                        tan4 = global_transform * tan4;
                        math::njVec3f new_tan = { tan4.x, tan4.y, tan4.z };
                        new_tan = math::normalize(new_tan);
                        v.tangent = { new_tan.x,
                                      new_tan.y,
                                      new_tan.z,
                                      v.tangent.w };
                    }
                    processed_primitives.emplace_back(transformed_vertices,
                                                      prim.indices,
                                                      prim.material_name);
                }

                meshes_.emplace_back(processed_name, processed_primitives);
            }

            for (int child : node.children) {
                self(self, child, global_transform);
            }
        };

        if (!root_nodes.empty()) {
            for (int root : root_nodes) {
                process_node(process_node, root, math::njMat4f::Identity());
            }
        } else {
            // Fallback
            for (const auto& raw : raw_meshes) {
                std::vector<core::njPrimitive> prims;
                for (const auto& rp : raw.primitives) {
                    prims.emplace_back(rp.vertices,
                                       rp.indices,
                                       rp.material_name);
                }
                meshes_.emplace_back(raw.name, prims);
            }
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

    void
    GLTFAsset::process_node_hierarchy(int node_index,
                                      const math::njMat4f& parent_transform,
                                      const std::vector<Node>& nodes,
                                      std::vector<core::njMesh>& out_meshes) {
        // Unused
    }

}  // namespace njin::gltf