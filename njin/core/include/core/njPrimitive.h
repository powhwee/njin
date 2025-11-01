#pragma once
#include <vector>

#include "core/njVertex.h"

namespace njin::core {
    class njPrimitive {
        public:
        njPrimitive(const std::vector<njVertex>& vertices, const std::vector<uint32_t>& indices);

        std::vector<njVertex> get_vertices() const;
        std::vector<uint32_t> get_indices() const;

        private:
        std::vector<njVertex> vertices_;
        std::vector<uint32_t> indices_;
    };

}  // namespace njin::core