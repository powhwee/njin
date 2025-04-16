#include "../include/util/file.h"

#include <iostream>

namespace njin::util {
    std::string read_file(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("Could not open file!");
        }

        size_t file_size = (size_t)file.tellg();
        std::string buffer{};
        buffer.resize(file_size);

        file.seekg(0);
        file.read(buffer.data(), file_size);
        file.close();
        return buffer;
    }
}  // namespace njin::util
