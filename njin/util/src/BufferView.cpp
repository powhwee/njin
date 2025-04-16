#include "util/BufferView.h"

namespace njin::gltf {

    BufferView::BufferView(const Buffer& buffer, const BufferViewInfo& info) :
        data_{ [&]() -> std::vector<std::byte> {
            return buffer.get_range(info.byte_offset,
                                    info.byte_length + info.byte_offset - 1);
        }() } {}

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
}  // namespace njin::gltf
