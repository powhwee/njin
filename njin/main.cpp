#include <chrono>
#include <cmath>
#include <ranges>
#include <thread>

#include <vulkan/image_setup.h>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "core/loader.h"
#include "ecs/njSceneLoader.h"
#include "core/njVertex.h"
#include "ecs/Components.h"
#include "ecs/njArchetype.h"
#include "ecs/njCameraArchetype.h"
#include "ecs/njEngine.h"
#include "ecs/njInputSystem.h"
#include "ecs/njMovementSystem.h"
#include "ecs/njPhysicsSystem.h"
#include "ecs/njPlayerArchetype.h"
#include "ecs/njRenderSystem.h"
#include "ecs/njSceneGraphSystem.h"
#include "math/njVec3.h"
#include "mnt/RoomBuilder.h"
#include "vulkan/AttachmentImages.h"
#include "vulkan/CommandBuffer.h"
#include "vulkan/DescriptorPoolBuilder.h"
#include "vulkan/DescriptorSetLayoutBuilder.h"
#include "vulkan/DescriptorSets.h"
#include "vulkan/GraphicsPipelineBuilder.h"
#include "vulkan/ImageBuilder.h"
#include "vulkan/PhysicalDevice.h"
#include "vulkan/PipelineLayoutBuilder.h"
#include "vulkan/RenderResources.h"
#include "vulkan/Renderer.h"
#include "vulkan/SamplerBuilder.h"
#include "vulkan/ShaderModule.h"
#include "vulkan/VertexBuffers.h"
#include "vulkan/Window.h"
#include "vulkan/config.h"
#include "vulkan/include/vulkan/RenderInfos.h"
#include "vulkan/pipeline_setup.h"
#include "vulkan/util.h"

#include <algorithm>
#include <iostream>

using namespace njin::vulkan;
using namespace njin;

constexpr int MAX_LIGHTS = 10;

namespace {}  // namespace

int main() {
    try {
        std::cout << "Starting njin..." << std::endl;
        namespace core = njin::core;
        
        SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);
        SDL_SetMainReady();

        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
            return 1;
        }
        std::cout << "SDL initialized." << std::endl;

        Window window{
            "njin",
            1280,
            720,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE,
        };
        std::cout << "Window created." << std::endl;

        auto extensions{ window.get_extensions() };
        Instance instance{ "njin", extensions };
        std::cout << "Instance created." << std::endl;

        Surface surface{ instance, window };
        PhysicalDevice physical_device{ instance, surface };
        
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physical_device.get(), &props);
        std::cout << "Physical device selected: " << props.deviceName << std::endl;

        LogicalDevice logical_device{ physical_device };
        std::cout << "Logical device created." << std::endl;

        Swapchain swapchain{ logical_device, physical_device, surface };

        RenderResourceInfos resource_infos{
            .attachment_images = { ATTACHMENT_IMAGE_INFO_DEPTH },
            .set_layouts = { DESCRIPTOR_SET_LAYOUT_INFO_MVP,
                             DESCRIPTOR_SET_LAYOUT_TEXTURES },
            .render_passes = { RENDER_PASS_INFO_MAIN },
            .pipelines = { PIPELINE_INFO_MAIN_DRAW },
            .vertex_buffers = { VERTEX_BUFFER_INFO_MAIN_DRAW },
            .index_buffers = { INDEX_BUFFER_INFO_MAIN_DRAW }
        };

        RenderResources resources{ logical_device,
                                   physical_device,
                                   swapchain,
                                   resource_infos };
        std::cout << "RenderResources created." << std::endl;

        Renderer renderer{ logical_device,
                           physical_device,
                           swapchain,
                           RENDERER_INFO,
                           resources };
        std::cout << "Renderer created." << std::endl;

        // Prepare registries
        core::njRegistry<core::njMesh> mesh_registry{};
        core::njRegistry<core::njMaterial> material_registry{};
        core::njRegistry<core::njTexture> texture_registry{};

        // Load assets from glTF files specified in the manifest
        std::cout << "Loading meshes..." << std::endl;
        load_meshes("main.meshes", mesh_registry, material_registry, texture_registry);

        // Load standalone textures for overrides
        std::cout << "Loading textures..." << std::endl;
        load_textures("main.textures", texture_registry);

        // Initialize engine and add all the systems we want
        ecs::njEngine engine{};
        bool should_run{ true };
        engine.add_system(std::make_unique<ecs::njInputSystem>(should_run));
        engine.add_system(std::make_unique<ecs::njMovementSystem>());
        engine.add_system(std::make_unique<ecs::njPhysicsSystem>());
        core::RenderBuffer render_buffer{};
        engine.add_system(std::make_unique<ecs::njRenderSystem>(render_buffer, mesh_registry, material_registry, texture_registry));

        // Load scene from configuration file
        ecs::njSceneLoader scene_loader{ "main.scene", mesh_registry, material_registry, texture_registry };
        scene_loader.load(engine);

        auto start_time = std::chrono::high_resolution_clock::now();
        std::cout << "Entering main loop..." << std::endl;

        int frame_count = 0;
        while (should_run) {
            if (frame_count++ % 100 == 0) std::cout << "Frame: " << frame_count << std::endl;
            
            auto camera_views = engine.get_view<njin::ecs::njTransformComponent, njin::ecs::njCameraComponent>();
            if (camera_views.empty()) {
                std::cerr << "No camera found!" << std::endl;
                break;
            }
            auto camera_transform_component = std::get<0>(camera_views[0].second);

            auto current_time = std::chrono::high_resolution_clock::now();
            float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

            float angle = time * 0.5f;  // Slower rotation
            float radius = 15.0f;
            float new_x = radius * cos(angle);
            float new_z = radius * sin(angle);

            camera_transform_component->transform[0][3] = new_x;
            camera_transform_component->transform[1][3] = 4.f;  // At model's mid-height
            camera_transform_component->transform[2][3] = new_z;

            engine.update();
            vulkan::RenderInfos render_queue{ mesh_registry,
                                              texture_registry,
                                              resources,
                                              render_buffer };
            renderer.draw_frame(render_queue);
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception caught in main." << std::endl;
        return 1;
    }
}
