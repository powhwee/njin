#include "vulkan/ShaderModule.h"

#include <format>

#include "glslang/Public/ShaderLang.h"
#include <fstream>
#include <iostream>
#include <vulkan/spirv.h>

namespace {
    using namespace njin::vulkan;

    /**
     *  Read a file
     * @param filename absolute path to file
     * @return Character list
     */
    std::string read_file(const std::string& filename);

    /**
     * Make a VkShaderModuleCreateInfo
     * @param Absolute filepath to shader source file
     * @param stage GLSL shader stage this shader is for
     * @return VkShaderModuleCreateInfo
     */
    VkShaderModuleCreateInfo make_shader_module_create_info(
        const std::string& shader_path,
        glslang_stage_t stage);

    std::string read_file(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        // since the position started at the end the current position is
        // the file size
        size_t file_size{ (size_t) file.tellg() };

        // set back to beginning and start to read
        file.seekg(0, std::ios::beg);

        std::string result{};
        result.resize(file_size);
        file.read(result.data(), file_size);

        return result;
    }


    VkShaderModuleCreateInfo make_shader_module_create_info(
        const std::string& shader_path,
        glslang_stage_t stage) {
        const std::string source{ read_file(shader_path) };


        SpirVBinary code{
            compileShaderToSPIRV_Vulkan(stage,
                                        source.data(),
                                        shader_path.data())
        };


        VkShaderModuleCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pNext = nullptr;
        // glslang compiles to int32_t words but vulkan wants
        // size in bytes
        info.codeSize = code.size * 4;
        info.pCode = code.words;

        return info;
    }
}


namespace njin::vulkan {
    ShaderModule::ShaderModule(const LogicalDevice& device,
                               const std::string& shader_path,
                               VkShaderStageFlagBits stage) :
        device_(device.get()),
        stage_{ stage } {
        // find corresponding glslang_stage_t; glslang compiler needs this info
        glslang_stage_t glslang_stage{};
        switch (stage_) {
            case VK_SHADER_STAGE_VERTEX_BIT:
                glslang_stage = GLSLANG_STAGE_VERTEX;
                break;
            case VK_SHADER_STAGE_FRAGMENT_BIT:
                glslang_stage = GLSLANG_STAGE_FRAGMENT;
                break;
            default:
                throw std::runtime_error("Shader module stage not supported");
        }

        const VkShaderModuleCreateInfo info{
            make_shader_module_create_info(shader_path, glslang_stage)
        };

        if (vkCreateShaderModule(device_, &info, nullptr, &shader_module_)) {
            throw std::runtime_error("failed to create shader module!");
        }
    }

    ShaderModule::~ShaderModule() {
        vkDestroyShaderModule(device_, shader_module_, nullptr);
    }

    VkShaderModule ShaderModule::get() const {
        return shader_module_;
    }

    VkShaderStageFlagBits ShaderModule::get_stage() const {
        return stage_;
    }

    const char* ShaderModule::get_entry_point() const {
        return entry_point_.data();
    }
}
