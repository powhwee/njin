#include "core/njNode.h"

#include <catch2/catch_test_macros.hpp>

#include "core/njMesh.h"

namespace njin::core {
    TEST_CASE("njNode", "[core][njNode]") {
        SECTION("construction") {
            SECTION("no throw") {
                njMesh mesh{};
                math::njMat4f transform{};
                std::vector<njNode> children{};

                REQUIRE_NOTHROW(mesh, transform, children);
            }
            SECTION("construction with provided local transform") {
                REQUIRE_NOTHROW(njNode{ "node", math::njMat4f::Identity() });
                njNode node{ "node", math::njMat4f::Identity() };

                // local transform is as set
                math::njMat4f local_transform{ node.get_local_transform() };
                REQUIRE(local_transform == math::njMat4f::Identity());

                // global transform is also as set, because the node
                // does not have a parent yet
                math::njMat4f global_transform{ node.get_global_transform() };
                REQUIRE(global_transform == math::njMat4f::Identity());
            }
            SECTION("construction without provided local transform") {
                njNode node{ "node" };

                // local transform defaults to identity
                math::njMat4f local_transform{ node.get_local_transform() };
                REQUIRE(local_transform == math::njMat4f::Identity());
            }
        }
        SECTION("hierarchy manipulation") {
            njNode root{ "root" };
            njNode child_0{ "child_0" };
            njNode child_1{ "child_1" };

            // add child 0
            REQUIRE_NOTHROW(root.add_child(&child_0));
            REQUIRE(root.get_children().size() == 1);

            // add child 1
            REQUIRE_NOTHROW(root.add_child(&child_1));
            REQUIRE(root.get_children().size() == 2);

            // remove child 0
            REQUIRE_NOTHROW(root.remove_child(&child_0));
            REQUIRE(root.get_children().size() == 1);

            // remove child 1
            REQUIRE_NOTHROW(root.remove_child(&child_1));
            REQUIRE(root.get_children().size() == 0);

            REQUIRE_NOTHROW(root.add_child(&child_0));
            REQUIRE(root.get_children().size() == 1);

            // try to remove child_1 when actually only child_0 is present
            REQUIRE_NOTHROW(root.remove_child(&child_1));
            REQUIRE(root.get_children().size() == 1);

            // removing nullptr
            REQUIRE_NOTHROW(root.remove_child(nullptr));
        }

        SECTION("transforms") {
            math::njMat4f two{ { 2.f, 2.f, 2.f, 2.f },
                               { 2.f, 2.f, 2.f, 2.f },
                               { 2.f, 2.f, 2.f, 2.f },
                               { 2.f, 2.f, 2.f, 2.f } };

            math::njMat4f three{ { 3.f, 3.f, 3.f, 3.f },
                                 { 3.f, 3.f, 3.f, 3.f },
                                 { 3.f, 3.f, 3.f, 3.f },
                                 { 3.f, 3.f, 3.f, 3.f } };

            njNode root{ "root" };
            REQUIRE(root.get_local_transform() == math::njMat4f::Identity());
            REQUIRE(root.get_global_transform() == math::njMat4f::Identity());

            njNode child_0{ "child_0", two };
            REQUIRE(child_0.get_local_transform() == two);
            REQUIRE(child_0.get_global_transform() == two);

            njNode child_1{ "child_1", three };
            REQUIRE(child_1.get_local_transform() == three);
            REQUIRE(child_1.get_global_transform() == three);

            root.add_child(&child_0);
            // local transform did not change
            REQUIRE(child_0.get_local_transform() == two);
            // global transform updated
            REQUIRE(child_0.get_global_transform() ==
                    math::njMat4f::Identity() * two);

            // update local transform of root
            root.set_local_transform(two);
            REQUIRE(root.get_local_transform() == two);
            REQUIRE(root.get_global_transform() == two);

            // local transform did not change
            REQUIRE(child_0.get_local_transform() == two);
            // global transform updated
            REQUIRE(child_0.get_global_transform() == two * two);

            child_0.add_child(&child_1);
            // local transform did not change
            REQUIRE(child_1.get_local_transform() == three);
            // global transform updated
            REQUIRE(child_1.get_global_transform() == two * two * three);

            child_0.remove_child(&child_1);
            // local transform still doesn't change
            REQUIRE(child_1.get_local_transform() == three);
            // global transform now reverts
            REQUIRE(child_1.get_global_transform() == three);
        }
    }
}  // namespace njin::core
