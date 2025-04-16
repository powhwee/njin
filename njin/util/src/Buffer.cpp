#include "util/Buffer.h"

namespace njin::gltf {
    Buffer::Buffer(std::istream& glb,
                   std::streampos chunk_start,
                   uint32_t chunk_size) {
        glb.seekg(chunk_start);
        data_.resize(chunk_size);
        glb.read(reinterpret_cast<char*>(data_.data()), chunk_size);
    }

    std::vector<std::byte> Buffer::get_range(uint32_t start,
                                             uint32_t end) const {
        std::vector<std::byte> copy{};
        for (uint32_t i{ start }; i <= end; ++i) {
            copy.push_back(data_.at(i));
        }
        return copy;
    }
}  // namespace njin::gltf
