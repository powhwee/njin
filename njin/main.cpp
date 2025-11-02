#include <chrono>
#include <cmath>
#include <ranges>
#include <thread>

#include <vulkan/image_setup.h>

#include "core/loader.h"
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

using namespace njin::vulkan;
using namespace njin;

constexpr int MAX_LIGHTS = 10;

namespace {}  // namespace

int main() {
    namespace core = njin::core;

    SDL_InitSubSystem(SDL_INIT_VIDEO);

    Window window{
        "njin",
        1280,
        720,
        SDL_WINDOW_VULKAN,
    };
    auto extensions{ window.get_extensions() };
    Instance instance{ "njin", extensions };
    Surface surface{ instance, window };
    PhysicalDevice physical_device{ instance, surface };
    LogicalDevice logical_device{ physical_device };

    Swapchain swapchain{ logical_device, physical_device, surface };

    RenderResourceInfos resource_infos{
        .attachment_images = { ATTACHMENT_IMAGE_INFO_DEPTH },
        .set_layouts = { DESCRIPTOR_SET_LAYOUT_INFO_MVP,
                         DESCRIPTOR_SET_LAYOUT_TEXTURES },
        .render_passes = { RENDER_PASS_INFO_MAIN, RENDER_PASS_INFO_ISO },
        .pipelines = { PIPELINE_INFO_MAIN_DRAW, PIPELINE_INFO_ISO_DRAW },
        .vertex_buffers = { VERTEX_BUFFER_INFO_MAIN_DRAW,
                            VERTEX_BUFFER_INFO_ISO_DRAW },
        .index_buffers = { INDEX_BUFFER_INFO_MAIN_DRAW }
    };

    RenderResources resources{ logical_device,
                               physical_device,
                               swapchain,
                               resource_infos };
    Renderer renderer{ logical_device,
                       physical_device,
                       swapchain,
                       RENDERER_INFO,
                       resources };

    // prepare meshes from .meshes file
    core::njRegistry<core::njMesh> mesh_registry{};
    load_meshes("main.meshes", mesh_registry);

    // prepare textures from .textures file
    core::njRegistry<core::njTexture> texture_registry{};
    load_textures("main.textures", texture_registry);

    // initialize engine and add all the systems we want
    ecs::njEngine engine{};
    bool should_run{ true };
    engine.add_system(std::make_unique<ecs::njInputSystem>(should_run));
    engine.add_system(std::make_unique<ecs::njMovementSystem>());
    core::RenderBuffer render_buffer{};
    engine.add_system(std::make_unique<ecs::njRenderSystem>(render_buffer));

    ecs::PerspectiveCameraSettings camera_settings{ .near = 1.f, .far = 1000.f, .horizontal_fov = 90.f };
    ecs::njCameraArchetypeCreateInfo camera_info{
        .name = "camera",
        .transform = ecs::njTransformComponent::make(10.f, 8.f, 10.f),
        .camera = { .type = ecs::njCameraType::Perspective,
                    .up = { 0.f, 1.f, 0.f },
                    .look_at = { 0.f, 0.f, 0.f },
                    .aspect = { 16.f / 9.f },
                    .settings = camera_settings }
    };

    ecs::njCameraArchetype camera_archetype{ camera_info };
    engine.add_archetype(camera_archetype);

    ecs::njObjectArchetypeCreateInfo object_info{
        .name = "cube",
        .transform = ecs::njTransformComponent::make(0.f, 0.f, 0.f),
        .mesh = { .mesh = "cube", .texture = "" }
    };
    ecs::njObjectArchetype object_archetype{ object_info };
    engine.add_archetype(object_archetype);

    // ecs::njInputComponent input{};

    // ecs::njPlayerArchetypeCreateInfo player_archetype_info{
    //     .name = "player",
    //     .transform = ecs::njTransformComponent::make(0.f, 1.f, 0.f),
    //     .input = {},
    //     .mesh = { .mesh = "player", .texture = "statue" },
    //     .intent = {},
    //     .physics = {}
    // };
    // ecs::njPlayerArchetype player_archetype{ player_archetype_info };
    // engine.add_archetype(player_archetype);

    auto start_time = std::chrono::high_resolution_clock::now();

    while (should_run) {
        auto camera_views = engine.get_view<njin::ecs::njTransformComponent, njin::ecs::njCameraComponent>();
        auto camera_transform_component = std::get<0>(camera_views[0].second);

        auto current_time = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(current_time - start_time).count();

        float angle = time * 0.5f;
        float radius = 14.0f;
        float new_x = radius * cos(angle);
        float new_z = radius * sin(angle);

        camera_transform_component->transform[0][3] = new_x;
        camera_transform_component->transform[1][3] = 8.0f;
        camera_transform_component->transform[2][3] = new_z;

        engine.update();
        vulkan::RenderInfos render_queue{ mesh_registry,
                                          texture_registry,
                                          resources,
                                          render_buffer };
        renderer.draw_frame(render_queue);
        // return 0;
    }
}
