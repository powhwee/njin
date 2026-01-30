#include "core/njTexture.h"

namespace njin::core {

    njTexture::njTexture(const njTextureCreateInfo& data) :
        name{ data.name },
        data_{ data.data },
        width_{ data.width },
        height_{ data.height },
        size_{ [data] {
            return static_cast<int>(data.width * data.height *
                                    sizeof(unsigned int));
        }() },
        channels_{ data.channels } {}

    ImageData njTexture::get_data() const {
        return data_;
    }

    uint32_t njTexture::get_width() const {
        return width_;
    };

    uint32_t njTexture::get_height() const {
        return height_;
    }

    int njTexture::get_size() const {
        return size_;
    }

    uint64_t njTexture::get_size_uint64() const {
        return static_cast<uint64_t>(size_);
    }

}  // namespace njin::core