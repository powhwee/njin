#include "core/MeshBuilder.h"

namespace njin::core {

    MeshBuilder::MeshBuilder(const std::vector<uint16_t>& indices) :
        indices_{ indices } {};

    void MeshBuilder::add_position_attributes(const std::vector<math::njVec3f>&
                                              positions) {
        positions_ = positions;
    }

    void MeshBuilder::add_normal_attributes(const std::vector<math::njVec3f>&
                                            normals) {
        normals_ = normals;
    }

    void
    MeshBuilder::add_tangent_attributes(const std::vector<math::njVec4<float>>&
                                        tangents) {
        tangents_ = tangents;
    }

    void
    MeshBuilder::add_color_attributes(const std::vector<math::njVec4<uint16_t>>&
                                      colors) {
        colors_ = colors;
    }

    void MeshBuilder::add_texture_coordinates(const std::vector<math::njVec2f>&
                                              tex_coords) {
        tex_coords_ = tex_coords;
    }

    njMesh MeshBuilder::build() {
        // find out how many triangles in the mesh
        size_t triangle_count{ indices_.size() / 3 };
        std::vector<njPrimitive> primitives{};
        for (int i{ 0 }; i < triangle_count; ++i) {
            int offset{ i * 3 };
            // 1 triangle primitive = 3 attributes
            // one
            njVertexCreateInfo one{};
            if (!positions_.empty()) {
                one.position = positions_[indices_[offset]];
            }
            if (!normals_.empty()) {
                one.normal = normals_[indices_[offset]];
            }
            if (!tangents_.empty()) {
                one.tangent = tangents_[indices_[offset]];
            }
            if (!colors_.empty()) {
                one.color = colors_[indices_[offset]];
            }
            if (!tex_coords_.empty()) {
                one.tex_coord = tex_coords_[indices_[offset]];
            }

            // two
            njVertexCreateInfo two{};
            if (!positions_.empty()) {
                two.position = positions_[indices_[offset + 1]];
            }
            if (!normals_.empty()) {
                two.normal = normals_[indices_[offset + 1]];
            }
            if (!tangents_.empty()) {
                two.tangent = tangents_[indices_[offset + 1]];
            }
            if (!colors_.empty()) {
                two.color = colors_[indices_[offset + 1]];
            }
            if (!tex_coords_.empty()) {
                two.tex_coord = tex_coords_[indices_[offset + 1]];
            }

            // three
            njVertexCreateInfo three{};
            if (!positions_.empty()) {
                three.position = positions_[indices_[offset + 2]];
            }
            if (!normals_.empty()) {
                three.normal = normals_[indices_[offset + 2]];
            }
            if (!tangents_.empty()) {
                three.tangent = tangents_[indices_[offset + 2]];
            }
            if (!colors_.empty()) {
                three.color = colors_[indices_[offset + 2]];
            }
            if (!tex_coords_.empty()) {
                three.tex_coord = tex_coords_[indices_[offset + 2]];
            }

            njVertex vertex_1{ one };
            njVertex vertex_2{ two };
            njVertex vertex_3{ three };
            njPrimitive primitive{ { vertex_1, vertex_2, vertex_3 } };
            primitives.emplace_back(primitive);
        }

        njMesh mesh{ primitives };
        return mesh;
    }
}  // namespace njin::core