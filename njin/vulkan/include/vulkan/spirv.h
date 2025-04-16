#pragma once

#include <cstdint>
#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/resource_limits_c.h>

namespace njin::vulkan {
    typedef struct SpirVBinary {
        uint32_t* words; // SPIR-V words
        int size; // number of words in SPIR-V binary
    } SpirVBinary;

    SpirVBinary compileShaderToSPIRV_Vulkan(glslang_stage_t stage,
                                            const char* shaderSource,
                                            const char* fileName);
}
