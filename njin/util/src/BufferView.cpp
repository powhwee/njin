#include "util/BufferView.h"

namespace njin::gltf {

    BufferView::BufferView(const Buffer& buffer, const BufferViewInfo& info) :
        data_{ [&]() -> std::vector<std::byte> {
            return buffer.get_range(info.byte_offset,
                                    info.byte_length + info.byte_offset - 1);
        }() },
        byte_stride_{ info.byte_stride } {}

    std::vector<std::byte> BufferView::get_range(uint32_t start,
                                                 uint32_t end) const {
        std::vector<std::byte> copy{};
        for (uint32_t i{ start }; i <= end; ++i) {
            copy.push_back(data_.at(i));
        }
        return copy;
    }

    std::vector<std::byte> BufferView::get() const {
        return data_;
    }

    std::optional<int> BufferView::get_byte_stride() const {
        return byte_stride_;
    }
}  // namespace njin::gltf
