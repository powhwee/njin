#pragma once
#include <string>
#include <vector>

#include "core/njVertex.h"

namespace njin::core {
    class njPrimitive {
        public:
        njPrimitive(const std::vector<njVertex>& vertices, const std::vector<uint32_t>& indices, const std::string& material_name);

        std::vector<njVertex> get_vertices() const;
        std::vector<uint32_t> get_indices() const;
        std::string get_material_name() const;

        private:
        std::vector<njVertex> vertices_;
        std::vector<uint32_t> indices_;
        std::string material_name_;
    };

}  // namespace njin::core