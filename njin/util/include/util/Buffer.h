#pragma once
#include <istream>
#include <vector>

namespace njin::gltf {
    /**
    * Container for chunk 2 of a GLB, where the binary data that the JSON chunk
    * (chunk 1) references is stored
    */
    class Buffer {
        public:
            /**
        * @param glb Stream of GLB binary data
        * @param chunk_start Start position of GLB binary chunk, as a byte offset
        * from the start of the GLB
        * @param chunk_size Size of the GLB binary chunk in bytes
        */
            Buffer(std::istream& glb,
                   std::streampos chunk_start,
                   uint32_t chunk_size);

            /**
         * Retrieve a copy of a range of bytes in the buffer
         * @param start Position of first byte to copy
         * @param end Position of last byte to copy (inclusive)
         * @return Copied bytes
         */
            std::vector<std::byte> get_range(uint32_t start,
                                             uint32_t end) const;

            Buffer() = default;

        private:
            std::vector<std::byte> data_{};
    };
}  // namespace njin::gltf
