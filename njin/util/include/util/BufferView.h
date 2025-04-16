#pragma once
#include <optional>

#include "util/Buffer.h"

namespace njin::gltf {
    enum class BufferViewTarget : int {
        UNKNOWN,
        ARRAY_BUFFER = 34962,
        ELEMENT_ARRAY_BUFFER = 34963
    };

    struct BufferViewInfo {
        int buffer{};
        int byte_offset{ 0 };
        int byte_length{};
        std::optional<int> byte_stride{};
        std::optional<int> target{};
        std::optional<std::string> name{};
    };

    /**
     * Representation of a glTF bufferView
     */
    class BufferView {
        public:
        /**
             * Constructor
             * @param buffer Buffer this bufferview is a view into
             * @param info Buffer view info
             */
        BufferView(const Buffer& buffer, const BufferViewInfo& info);

        BufferView() = default;

        /**
         * Retrieve a copy of a range of bytes in the bufferview
         * @param start Position of first byte to copy
         * @param end Position of last byte to copy (inclusive)
         * @return Copied bytes
         */
        std::vector<std::byte> get_range(uint32_t start, uint32_t end) const;

        /**
         * Return the entire array of bytes referenced by this buffer view
         * @return Array of bytes
         */
        std::vector<std::byte> get() const;

        private:
        std::vector<std::byte> data_{};
    };
}  // namespace njin::gltf
