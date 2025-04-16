#include "vulkan/Instance.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <SDL3/SDL_vulkan.h>

// namespace {
bool check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : validation_layers) {
        bool layer_found = false;
        for (const auto& layer_properties : available_layers) {
            if (strcmp(layer_name, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }
    return true;
}

/**
 * Builder for vkInstanceCreateInfo
 * @param application_info Application info
 * @param extensions Extensions the VkInstance requires
 * @param debug_create_info Debug create info - ignored if not on a debug build
 * @return Instance create info
 */
VkInstanceCreateInfo
make_instance_create_info(const VkApplicationInfo* application_info,
                          std::vector<const char*>& extensions,
                          const void* debug_create_info) {
    // printf shader debugging
    std::vector<VkValidationFeatureEnableEXT> validation_feature_enables = {
        VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
    };

    VkValidationFeaturesEXT validation_features{};
    validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures =
    validation_feature_enables.data();
    validation_features.pNext = debug_create_info;

    VkInstanceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.pApplicationInfo = application_info;

    // on debug builds we want one additional validation layer on top
    // of the main vulkan instance
    if (enable_validation_layers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        info.pNext = &validation_features;
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = validation_layers.data();
    } else {
        info.enabledLayerCount = 0;
        info.ppEnabledLayerNames = nullptr;
    }

    // extensions
    info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    info.ppEnabledExtensionNames = extensions.data();

    return info;
}

VkApplicationInfo make_application_info(const std::string& app_name) {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = app_name.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "njin";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_3;

    return app_info;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
               VkDebugUtilsMessageTypeFlagsEXT message_type,
               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
               void* pUserData) {
    std::cerr << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
};

VkDebugUtilsMessengerCreateInfoEXT make_debug_messenger_create_info() {
    VkDebugUtilsMessengerCreateInfoEXT info{};
    info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    info.pNext = nullptr;
    info.flags = 0;
    info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    info.pfnUserCallback = debug_callback;
    info.pUserData = nullptr;

    return info;
}

// }

namespace njin::vulkan {
    Instance::Instance(const std::string& app_name,
                       std::vector<const char*>& extensions) {
        if ((enable_validation_layers) && !check_validation_layer_support()) {
            throw std::runtime_error(
            "Validation layers requested but not available!");
        }

        VkApplicationInfo app_info{ make_application_info(app_name) };

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{
            make_debug_messenger_create_info()
        };

        // printf shader debugging
        std::vector<VkValidationFeatureEnableEXT> validation_feature_enables = {
            VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
        };

        VkValidationFeaturesEXT validation_features{};
        validation_features.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
        validation_features.enabledValidationFeatureCount = 1;
        validation_features.pEnabledValidationFeatures =
        validation_feature_enables.data();
        validation_features.pNext = &debug_create_info;

        VkApplicationInfo application_info{ make_application_info(app_name) };
        VkInstanceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pNext = nullptr;
        info.flags = 0;
        info.pApplicationInfo = &application_info;

        // on debug builds we want one additional validation layer on top
        // of the main vulkan instance
        if (enable_validation_layers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

            info.pNext = &validation_features;
            info.enabledLayerCount = 1;
            info.ppEnabledLayerNames = validation_layers.data();
        } else {
            info.enabledLayerCount = 0;
            info.ppEnabledLayerNames = nullptr;
        }

        // extensions
        info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        info.ppEnabledExtensionNames = extensions.data();

        if (vkCreateInstance(&info, nullptr, &instance_) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    Instance::~Instance() {
        vkDestroyInstance(instance_, nullptr);
    }

    const VkInstance* const Instance::get() const {
        return &instance_;
    }
}  // namespace njin::vulkan
