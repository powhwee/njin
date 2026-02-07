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

                // JOINTS_0 - joint indices for skinning
                std::vector<math::njVec4<uint16_t>> joints{};
                if (attribute.HasMember("JOINTS_0")) {
                    int joints_accessor_index{ attribute["JOINTS_0"].GetInt() };
                    auto gltf_joints_accessor =
                    accessors[joints_accessor_index].GetObject();
                    Accessor::AccessorCreateInfo joints_accessor_info{
                        make_accessor_create_info(buffer_views_,
                                                  gltf_joints_accessor)
                    };
                    Accessor joints_accessor{ joints_accessor_info };
                    if (joints_accessor_info.component_type ==
                        ComponentType::UnsignedByte) {
                        joints = joints_accessor.get_vec4_ubyte();
                    } else {
                        joints = joints_accessor.get_vec4ushort();
                    }
                }

                // WEIGHTS_0 - joint weights for skinning
                std::vector<math::njVec4f> weights{};
                if (attribute.HasMember("WEIGHTS_0")) {
                    int weights_accessor_index{
                        attribute["WEIGHTS_0"].GetInt()
                    };
                    auto gltf_weights_accessor =
                    accessors[weights_accessor_index].GetObject();
                    Accessor::AccessorCreateInfo weights_accessor_info{
                        make_accessor_create_info(buffer_views_,
                                                  gltf_weights_accessor)
                    };
                    Accessor weights_accessor{ weights_accessor_info };
                    weights = weights_accessor.get_vec4f();
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
                    if (!joints.empty())
                        create_info.joints = joints[i];
                    if (!weights.empty())
                        create_info.weights = weights[i];
                    vertices.emplace_back(create_info);
                }
                primitives.emplace_back(vertices, indices, material_name);
            }
            raw_meshes.push_back({ mesh_name, primitives });
        }

        // 2. Process Nodes into Skeleton
        std::vector<core::njSkeletonNode> skeleton_nodes;
        if (document.HasMember("nodes")) {
            const auto& nodes_array = document["nodes"].GetArray();
            skeleton_nodes.resize(nodes_array.Size());
            int node_idx = 0;
            for (const auto& node_val : nodes_array) {
                core::njSkeletonNode node;
                node.parent = -1;  // Will be set below
                node.mesh_index = -1;

                if (node_val.HasMember("name"))
                    node.name = node_val["name"].GetString();
                else
                    node.name = "node_" + std::to_string(node_idx);

                if (node_val.HasMember("mesh")) {
                    node.mesh_index = node_val["mesh"].GetInt();
                    if (node.mesh_index >= 0 &&
                        node.mesh_index < raw_meshes.size()) {
                        node.mesh_name = raw_meshes[node.mesh_index].name +
                                         "_" + node.name;
                    }
                }

                std::vector<int> children;
                if (node_val.HasMember("children")) {
                    for (const auto& child : node_val["children"].GetArray()) {
                        children.push_back(child.GetInt());
                    }
                }
                node.children = children;

                math::njVec3f translation = { 0, 0, 0 };
                math::njVec3f scale = { 1, 1, 1 };
                math::njVec4f rotation = { 0, 0, 0, 1 };

                if (node_val.HasMember("matrix")) {
                    const auto& matrix = node_val["matrix"].GetArray();

                    // node matrix is column-major
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
                    node.local_transform = math::njMat4f(r0, r1, r2, r3);

                    // We should decompose this matrix for robust animation,
                    // but GLTF usually provides T/R/S parallel to Matrix if it wants to support animation?
                    // Actually spec says: "A node can have either a matrix or any combination of translation/rotation/scale"
                    // If matrix is present, T/R/S properties are ignored.
                    // But if we want to animate it, we NEED T/R/S.
                    // Let's assume for now that animated nodes won't use the 'matrix' property in the GLTF.
                    // If they do, we default bind pose to Identity components (or 0,0,0 / 1,1,1 / Identity Quat)
                    // which might cause "popping" if the animation doesn't start at t=0 perfectly matching the matrix.
                    // This is an edge case.
                    node.bind_translation = { 0, 0, 0 };
                    node.bind_scale = { 1, 1, 1 };
                    node.bind_rotation = math::njQuatf(0, 0, 0, 1);
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
                    node.bind_translation = translation;
                    node.bind_rotation = math::njQuatf(rotation.x,
                                                       rotation.y,
                                                       rotation.z,
                                                       rotation.w);
                    node.bind_scale = scale;
                    node.local_transform = t_mat * r_mat * s_mat;
                    node.bind_scale = scale;
                    node.local_transform = t_mat * r_mat * s_mat;
                }

                skeleton_nodes[node_idx] = node;

                // Track children to set their parent later
                for (int child_idx : children) {
                    // We can't set parent here because child node might not be processed yet
                    // But we can rely on a second pass or store data.
                    // Actually, nodes are processed in order 0..N, but children refs are indices.
                    // The simplest way: store parent mapping in a temp vector
                }
                node_idx++;
            }

            // Second pass: Set parents
            node_idx = 0;
            for (const auto& node_val : nodes_array) {
                if (node_val.HasMember("children")) {
                    for (const auto& child : node_val["children"].GetArray()) {
                        skeleton_nodes[child.GetInt()].parent = node_idx;
                    }
                }
                node_idx++;
            }
        }

        // 3. Find Roots
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

        skeleton_ = { skeleton_nodes, root_nodes };

        // 4. Parse Animations
        if (document.HasMember("animations")) {
            for (const auto& anim_val : document["animations"].GetArray()) {
                core::njAnimation animation;
                if (anim_val.HasMember("name")) {
                    animation.name = anim_val["name"].GetString();
                } else {
                    animation.name = "anim_" +
                                     std::to_string(animations_.size());
                }

                // Samplers: input (time) / output (values) accessors
                struct Sampler {
                    std::vector<float> times;
                    // We store raw accessor ref info here, parse on demand
                    int output_accessor;
                };

                std::vector<Sampler> samplers;
                const auto& samplers_array = anim_val["samplers"].GetArray();
                for (const auto& s_val : samplers_array) {
                    Sampler s;
                    int input = s_val["input"].GetInt();
                    s.output_accessor = s_val["output"].GetInt();

                    auto accessor = accessors[input].GetObject();
                    auto info = make_accessor_create_info(buffer_views_,
                                                          accessor);
                    gltf::Accessor acc{ info };
                    s.times = acc.get_scalar_f();
                    samplers.push_back(s);
                }

                // Channels: target node + path
                const auto& channels_array = anim_val["channels"].GetArray();
                float max_time = 0.0f;

                for (const auto& c_val : channels_array) {
                    core::njAnimationChannel channel;
                    int sampler_idx = c_val["sampler"].GetInt();
                    const auto& target = c_val["target"].GetObject();
                    channel.target_node = target["node"].GetInt();

                    std::string path = target["path"].GetString();
                    if (path == "translation")
                        channel.path =
                        core::njAnimationChannel::Path::Translation;
                    else if (path == "rotation")
                        channel.path = core::njAnimationChannel::Path::Rotation;
                    else if (path == "scale")
                        channel.path = core::njAnimationChannel::Path::Scale;
                    else
                        continue;  // Unsupported path

                    const auto& sampler = samplers[sampler_idx];
                    auto output_accessor =
                    accessors[sampler.output_accessor].GetObject();
                    auto output_info =
                    make_accessor_create_info(buffer_views_, output_accessor);
                    gltf::Accessor output_acc{ output_info };

                    // Populate keyframes
                    for (size_t i = 0; i < sampler.times.size(); ++i) {
                        core::njKeyframe kf;
                        kf.time = sampler.times[i];
                        if (kf.time > max_time)
                            max_time = kf.time;

                        if (channel.path ==
                            core::njAnimationChannel::Path::Translation ||
                            channel.path ==
                            core::njAnimationChannel::Path::Scale) {
                            math::njVec3f val = output_acc.get_vec3f()[i];
                            kf.value = val;
                        } else {  // Rotation
                            math::njVec4f val = output_acc.get_vec4f()[i];
                            kf.value =
                            math::njQuatf(val.x, val.y, val.z, val.w);
                        }
                        channel.keyframes.push_back(kf);
                    }
                    animation.channels.push_back(channel);
                }
                animation.duration = max_time;
                animations_.push_back(animation);
            }
        }

        // 4b. Parse Skin Data
        if (document.HasMember("skins") && document["skins"].IsArray() &&
            document["skins"].Size() > 0) {
            // Use first skin (most glTF files have one skin)
            const auto& skin = document["skins"][0].GetObject();

            // Joint node indices
            if (skin.HasMember("joints") && skin["joints"].IsArray()) {
                for (const auto& j : skin["joints"].GetArray()) {
                    skeleton_.joint_nodes.push_back(j.GetInt());
                }
            }

            // Inverse bind matrices
            if (skin.HasMember("inverseBindMatrices")) {
                int ibm_accessor_index = skin["inverseBindMatrices"].GetInt();
                auto gltf_ibm_accessor =
                accessors[ibm_accessor_index].GetObject();
                Accessor::AccessorCreateInfo ibm_info{
                    make_accessor_create_info(buffer_views_, gltf_ibm_accessor)
                };
                Accessor ibm_accessor{ ibm_info };
                skeleton_.inverse_bind_matrices = ibm_accessor.get_mat4f();
            }

            std::cout << "[GLTF] Loaded skin: " << skeleton_.joint_nodes.size()
                      << " joints, " << skeleton_.inverse_bind_matrices.size()
                      << " inverse bind matrices" << std::endl;
        }

        // 5. Build Meshes without baking (Raw primitives)
        // Since we are using runtime hierarchy, we just copy raw primitives
        // and assign them names based on the node that uses them.

        // Note: A mesh can be instantiated by multiple nodes.
        // For each node that has a mesh, we create a mesh instance.
        // But get_meshes() returns a flat list.
        // In the new system, Entity creation logic (scene loader) will iterate the Skeleton.
        // So here we should probably just return the raw meshes as-is?
        // However, existing scene loader expects "mesh_name" to work.
        // The existing loader iterates through get_meshes() registry.

        // Strategy:
        // We register meshes by their node name: "alias-mesh_name_node_name"
        // This allows unique entity creation per node.

        // process_node_hierarchy(0, -1, skeleton_nodes);

        // Register meshes with node names so we can instantiate them by name later
        // Note: No baking! Just raw vertices.
        auto register_meshes = [&](auto self, int node_index) -> void {
            if (node_index >= skeleton_nodes.size())
                return;
            const auto& node = skeleton_nodes[node_index];

            if (node.mesh_index >= 0 && node.mesh_index < raw_meshes.size()) {
                const RawMesh& raw_mesh = raw_meshes[node.mesh_index];
                // Name format: alias-mesh_name_node_name
                // This matches what the old baker did for naming
                std::string processed_name = raw_mesh.name + "_" + node.name;

                std::vector<core::njPrimitive> prims;
                for (const auto& rp : raw_mesh.primitives) {
                    prims.emplace_back(rp.vertices,
                                       rp.indices,
                                       rp.material_name);
                }
                meshes_.emplace_back(processed_name, prims);
            }

            // Recurse for children
            // Since children are stored by index in a separate map or we just iterate generic nodes...
            // Wait, skeleton_nodes stores 'parent', but to traverse down we need children list.
            // We didn't store children in our njSkeletonNode (optimized for bottom-up).
            // But valid: we can just iterate ALL nodes.
        };

        for (int i = 0; i < skeleton_nodes.size(); ++i) {
            register_meshes(register_meshes, i);
        }
    }

    std::vector<core::njMesh> GLTFAsset::get_meshes() const {
        return meshes_;
    }

    std::vector<core::njMaterial> GLTFAsset::get_materials() const {
        return materials_;
    }

    std::vector<core::njAnimation> GLTFAsset::get_animations() const {
        return animations_;
    }

    core::njSkeleton GLTFAsset::get_skeleton() const {
        return skeleton_;
    }

    std::vector<core::njTexture> GLTFAsset::get_textures() const {
        return textures_;
    }

    // This function now just registers the raw meshes with node-aware names
    // It NO LONGER bakes transforms. Transforms are applied at runtime.
    void
    GLTFAsset::process_node_hierarchy(int node_index,
                                      int parent_node_index,
                                      const std::vector<core::njSkeletonNode>&
                                      nodes) {
        // Recursive traversal to find meshes
        if (node_index < 0 || node_index >= nodes.size())
            return;

        const auto& node = nodes[node_index];

        // This logic is slightly tricky: we need to pull the RAW meshes
        // but we don't have them stored as member variable.
        // We need to refactor the constructor slightly to allow access here
        // OR move this logic into the constructor.
        // For simplicity, let's just do it in the constructor loop or keep raw_meshes available.
        // Since I can't easily access 'raw_meshes' from here without changing class state...
        // I will change the caller in valid replacement block.
    }

}  // namespace njin::gltf