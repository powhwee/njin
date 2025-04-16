#pragma once
#include <glslang/Include/glslang_c_interface.h>

#include "vulkan/LogicalDevice.h"

namespace njin::vulkan {
    class ShaderModule {
    public:
        /**
         * Constructor
         * @param device Logical device
         * @param shader_path Absolute filepath to shader
         * @param @stage Stage this shader is for
         */
        ShaderModule(const LogicalDevice& device,
                     const std::string& shader_path,
                     VkShaderStageFlagBits stage);

        ShaderModule(const ShaderModule& other) = delete;

        ShaderModule& operator=(const ShaderModule& other) = delete;

        ShaderModule(ShaderModule&& other) = delete;

        ShaderModule& operator=(ShaderModule&& other) = delete;

        ~ShaderModule();

        /**
         * Get the VkShaderModule
         * @return VkShaderModule
         */
        VkShaderModule get() const;

        /**
         * Get the shader stage associated with this module
         * @return VkShaderStageFlagBits
         */
        VkShaderStageFlagBits get_stage() const;


        /**
         * Get the name of the entry point method for this modulle
         * @return Name of entry point method
         */
        const char* get_entry_point() const;

    private:
        VkDevice device_{ VK_NULL_HANDLE };
        VkShaderModule shader_module_{ VK_NULL_HANDLE };
        VkShaderStageFlagBits stage_;
        std::string entry_point_{ "main" };
    };
}
