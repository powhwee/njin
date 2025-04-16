#pragma once
#include <vector>

#include "core/njVertex.h"

namespace njin::core {
    class njPrimitive {
        public:
        njPrimitive(const std::array<njVertex, 3>& vertices);

        std::vector<njVertex> get_vertices() const;

        private:
        std::array<njVertex, 3> vertices_;
    };

}  // namespace njin::core