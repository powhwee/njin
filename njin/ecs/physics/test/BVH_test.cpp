#include "physics/BVH.h"

#include <catch2/catch_test_macros.hpp>

namespace njin::ecs::physics {
    TEST_CASE("bvh", "[ecs][physics][BVH]") {
        BoundingBox box_0{
            .centroid = { 0.5f, 0.5f, 0.5f },
            .min_x = 0.f,
            .max_x = 1.f,
            .min_y = 0.f,
            .max_y = 1.f,
            .min_z = 0.f,
            .max_z = 1.f,
        };
        BoundingBox box_1{
            .centroid = { 0.f, 0.f, 0.f },
            .min_x = -0.5f,
            .max_x = 0.5f,
            .min_y = -0.5f,
            .max_y = 0.5f,
            .min_z = -0.5f,
            .max_z = 0.5f,
        };

        Primitive prim_0{ 0, box_0 };
        Primitive prim_1{ 1, box_1 };
        SECTION("construction") {
            SECTION("no nodes") {
                BVH bvh{ {} };
                const BVHNode* root{ bvh.get_root() };
                REQUIRE(!root);
            }
            SECTION("one node") {
                BVH bvh{ { prim_0 } };
                const BVHNode* root{ bvh.get_root() };
                REQUIRE(root);
                REQUIRE(root->is_leaf());
                REQUIRE(root->get_entities().size() == 1);
            }

            SECTION("two nodes") {
                BVH bvh{ { prim_0, prim_1 } };
                const BVHNode* root{ bvh.get_root() };
                REQUIRE(root);
                REQUIRE(!root->is_leaf());

                const BVHNode* left{ root->get_left() };
                const BVHNode* right{ root->get_right() };

                REQUIRE(left);
                REQUIRE(right);

                REQUIRE(left->is_leaf());
                REQUIRE(left->get_entities().size() == 1);
                REQUIRE(right->is_leaf());
                REQUIRE(right->get_entities().size() == 1);

                REQUIRE(!left->get_left());
                REQUIRE(!right->get_right());
            }
        }
        SECTION("overlap") {
            BVH bvh{ { prim_0, prim_1 } };
            const BVHNode* root{ bvh.get_root() };

            REQUIRE(root->get_left()->does_overlap(*root->get_right()));
            REQUIRE(root->get_right()->does_overlap(*root->get_left()));

            std::vector<EntityId> expected_overlaps_zero{ 1 };
            REQUIRE(bvh.get_overlaps(0) == expected_overlaps_zero);

            std::vector<EntityId> expected_overlaps_one{ 0 };
            REQUIRE(bvh.get_overlaps(1) == expected_overlaps_one);
        }
    }

    TEST_CASE("3D bounding box", "[ecs][physics][BoundingBox]") {
        BoundingBox box_0{
            .centroid = { 0.5f, 0.5f, 0.5f },
            .min_x = 0.f,
            .max_x = 1.f,
            .min_y = 0.f,
            .max_y = 1.f,
            .min_z = 0.f,
            .max_z = 1.f,
        };

        BoundingBox box_1{
            .centroid = { -1.5f, -1.5f, -1.5f },
            .min_x = -2.f,
            .max_x = -1.f,
            .min_y = -2.f,
            .max_y = -1.f,
            .min_z = -2.f,
            .max_z = -1.f,
        };

        BoundingBox box_2{
            .centroid = { 0.f, 0.f, 0.f },
            .min_x = -0.5f,
            .max_x = 0.5f,
            .min_y = -0.5f,
            .max_y = 0.5f,
            .min_z = -0.5f,
            .max_z = 0.5f,
        };

        // box 3 is above box 2 but share the same x and z
        BoundingBox box_3{
            .centroid = { 0.f, 0.f, 0.f },
            .min_x = -0.5f,
            .max_x = 0.5f,
            .min_y = 10.f,
            .max_y = 10.5f,
            .min_z = -0.5f,
            .max_z = 0.5f,
        };

        SECTION("3D overlaps") {
            // box 0 doesn't overlap with box 1,
            // but it does overlap with box 2
            REQUIRE(!box_0.does_overlap(box_1, BoundingBoxType::XYZ));
            REQUIRE(box_0.does_overlap(box_2, BoundingBoxType::XYZ));

            // it doesn't overlap with box_3, which is too high up
            REQUIRE(!box_0.does_overlap(box_3, BoundingBoxType::XYZ));
        }

        SECTION("2D overlaps") {
            REQUIRE(box_0.does_overlap(box_2, BoundingBoxType::XZ));
            REQUIRE(box_0.does_overlap(box_3, BoundingBoxType::XZ));
        }
    }

    TEST_CASE("2D BVH", "[ecs][physics][BVH]") {
        BoundingBox box_0{
            BoundingBox::make({ 0.f, 0.f, 0.f }, 1.f, 0.f, 1.f)
        };
        BoundingBox box_1{
            BoundingBox::make({ 0.5f, 0.f, 0.5f }, 1.f, 0.f, 1.f)
        };

        // box 2 is the same as box 1 in the XZ plane, just way above it along the Y axis
        BoundingBox box_2{
            BoundingBox::make({ 0.5f, 10.f, 0.5f }, 1.f, 0.f, 1.f)
        };

        BoundingBox box_3{
            BoundingBox::make({ 10.f, 0.f, 10.f }, 1.f, 0.f, 1.f)
        };

        Primitive prim_0{ 0, box_0 };
        Primitive prim_1{ 1, box_1 };
        Primitive prim_2{ 2, box_2 };
        Primitive prim_3{ 3, box_3 };

        SECTION("overlap") {
            BVH bvh{ { prim_0, prim_1, prim_2, prim_3 }, BoundingBoxType::XZ };
            REQUIRE(bvh.get_entities().size() == 4);
            std::vector<EntityId> prim_0_overlaps{ bvh.get_overlaps(0) };

            // 0 overlaps with 1 and 2 (we don't care about Y axis)
            REQUIRE(prim_0_overlaps.size() == 2);

            std::vector<EntityId> prim_3_overlaps{ bvh.get_overlaps(3) };
            REQUIRE(prim_3_overlaps.size() == 0);
        }
    }
}  // namespace njin::ecs::physics
