#pragma once
#include <cstdint>
#include <vector>

#include "util/stb.h"

namespace njin::core {
    using ImageData = std::vector<stbi_uc>;
    enum class TextureChannels : uint8_t {
        Default = 0,  // only used for desired_channels
        Gray = 1,
        GrayA = 2,
        RGB = 3,
        RGBA = 4
    };

    struct njTextureCreateInfo {
        ImageData data;
        int width;
        int height;
        TextureChannels channels;
        std::string name;
    };

    class njTexture {
        public:
        explicit njTexture(const njTextureCreateInfo& data);
        ImageData get_data() const;

        uint32_t get_width() const;
        uint32_t get_height() const;
        int get_size() const;

        uint64_t get_size_uint64() const;
        
        std::string name;

        private:
        ImageData data_;
        const int width_;
        const int height_;
        const int size_;

        TextureChannels channels_;
    };
}  // namespace njin::core
