#include "util/GLTFAsset.h"

#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include <rapidjson/document.h>
#include <vulkan/vulkan_core.h>

#include "math/njVec3.h"

uint32_t MAGIC{ 0x46'54'6C'67 };
uint32_t VERSION{ 2 };
uint32_t JSON_CHUNK_TYPE{ 0x4E'4F'53'4A };
uint32_t BIN_CHUNK_TYPE{ 0x00'4E'49'42 };

namespace rj = rapidjson;

namespace {
    namespace gltf = njin::gltf;
    namespace math = njin::math;

    std::vector<gltf::BufferView>
    process_buffer_views(const gltf::Buffer& buffer,
                         const rj::Document& document) {
        std::vector<gltf::BufferView> result{};
        rj::GenericArray buffer_views{ document["bufferViews"].GetArray() };
        for (auto it{ buffer_views.begin() }; it != buffer_views.end(); ++it) {
            rj::GenericObject buffer_view{ it->GetObject() };

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
        } else if (component_type == static_cast<int>(ComponentType::Float)) {
            return ComponentType::Float;
        }
    }

    template<typename ValueT>
    gltf::Accessor::AccessorCreateInfo
    make_accessor_create_info(const std::vector<gltf::BufferView>& buffer_views,
                              const rj::GenericObject<false, ValueT>&
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
            byte_offset = accessor["byteOffset"].GetInt();
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

}  // namespace

namespace njin::gltf {

    GLTFAsset::GLTFAsset(const std::string& path) {
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

        rj::GenericArray meshes{ document["meshes"].GetArray() };
        rj::GenericObject mesh{ meshes[0].GetObject() };
        rj::GenericArray primitives{ mesh["primitives"].GetArray() };
        rj::GenericObject primitive{ primitives[0].GetObject() };
        rj::GenericObject attribute{ primitive["attributes"].GetObject() };

        rj::GenericArray accessors{ document["accessors"].GetArray() };

        // indices
        int indices_accessor_index{ primitive["indices"].GetInt() };
        rj::GenericObject gltf_indices_accessor{
            accessors[indices_accessor_index].GetObject()
        };
        gltf::Accessor::AccessorCreateInfo indices_accessor_info{
            make_accessor_create_info(buffer_views_, gltf_indices_accessor)
        };
        gltf::Accessor indices_accessor{ indices_accessor_info };
        indices_ = indices_accessor.get_scalar();

        // position accessor
        int position_accessor_index{ attribute["POSITION"].GetInt() };
        rj::GenericObject gltf_position_accessor{
            accessors[position_accessor_index].GetObject()
        };

        gltf::Accessor::AccessorCreateInfo position_accessor_info{
            make_accessor_create_info(buffer_views_, gltf_position_accessor)
        };

        gltf::Accessor position_accessor{ position_accessor_info };
        position_attributes_ = position_accessor.get_vec3f();

        // normals accessor
        int normal_accessor_index{ attribute["NORMAL"].GetInt() };
        rj::GenericObject gltf_normal_accessor{
            accessors[normal_accessor_index].GetObject()
        };

        gltf::Accessor::AccessorCreateInfo normal_accessor_info{
            make_accessor_create_info(buffer_views_, gltf_normal_accessor)
        };

        gltf::Accessor normal_accessor{ normal_accessor_info };
        normal_attributes_ = normal_accessor.get_vec3f();

        // texture coordinates
        int tex_coord_accessor_index{ attribute["TEXCOORD_0"].GetInt() };
        rj::GenericObject gltf_tex_coord_accessor{
            accessors[tex_coord_accessor_index].GetObject()
        };

        Accessor::AccessorCreateInfo tex_coord_accessor_info{
            make_accessor_create_info(buffer_views_, gltf_tex_coord_accessor)
        };
        Accessor tex_coord_accessor{ tex_coord_accessor_info };
        tex_coords_ = tex_coord_accessor.get_vec2f();

        // colors
        if (attribute.HasMember("COLOR_0")) {
            int color_accessor_index{ attribute["COLOR_0"].GetInt() };
            rj::GenericObject gltf_color_accessor{
                accessors[color_accessor_index].GetObject()
            };
            Accessor::AccessorCreateInfo color_accessor_info{
                make_accessor_create_info(buffer_views_, gltf_color_accessor)
            };
            Accessor color_accessor{ color_accessor_info };
            color_attributes_ = color_accessor.get_vec4ushort();
        }

        // tangents accessor
        // int tangent_accessor_index{ attribute["TANGENT"].GetInt() };
        // rj::GenericObject gltf_tangent_accessor{
        //     accessors[tangent_accessor_index].GetObject()
        // };

        // gltf::Accessor::AccessorCreateInfo tangent_accessor_info{
        //     make_accessor_create_info(buffer_views_, gltf_tangent_accessor)
        // };
        // gltf::Accessor tangent_accessor{ tangent_accessor_info };
    }

    std::vector<math::njVec3f> GLTFAsset::get_position_attributes() const {
        return position_attributes_;
    }

    std::vector<math::njVec3f> GLTFAsset::get_normal_attributes() const {
        return normal_attributes_;
    }

    std::vector<math::njVec4<float>> GLTFAsset::get_tangent_attributes() const {
        return tangent_attributes_;
    }

    std::vector<math::njVec2f> GLTFAsset::get_texture_coordinates() const {
        return tex_coords_;
    }

    std::vector<math::njVec4<uint16_t>>
    GLTFAsset::get_color_attributes() const {
        return color_attributes_;
    }

    std::vector<uint16_t> GLTFAsset::get_indices() const {
        return indices_;
    }
}  // namespace njin::gltf
