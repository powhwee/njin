#include "core/njPrimitive.h"

#include <catch2/catch_test_macros.hpp>

namespace njin::core {

    TEST_CASE("njPrimitive", "[core][njPrimitive]") {
        SECTION("constructor no throw") {
            njVertexCreateInfo info{};
            njVertex one{ info };
            njVertex two{ info };
            njVertex three{ info };

            REQUIRE_NOTHROW(njPrimitive{ { one, two, three } });
        }
    }
}  // namespace njin::core
