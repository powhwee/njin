#include <chrono>

#include <vulkan/image_setup.h>

#include "core/loader.h"
#include "core/njVertex.h"
#include "ecs/Components.h"
#include "ecs/nj2DPhysicsSystem.h"
#include "ecs/njArchetype.h"
#include "ecs/njEngine.h"
#include "ecs/njInputSystem.h"
#include "ecs/njMovementSystem.h"
#include "ecs/njRenderSystem.h"
#include "mnt/TownLevel.h"
#include "vulkan/AttachmentImages.h"
#include "vulkan/DescriptorSets.h"
#include "vulkan/PhysicalDevice.h"
#include "vulkan/RenderResources.h"
#include "vulkan/Renderer.h"
#include "vulkan/ShaderModule.h"
#include "vulkan/Window.h"
#include "vulkan/config.h"
#include "vulkan/include/vulkan/RenderInfos.h"

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
                         DESCRIPTOR_SET_LAYOUT_TEXTURES,
                         DESCRIPTOR_SET_LAYOUT_COLLIDERS },
        .render_passes = { RENDER_PASS_INFO_MAIN, RENDER_PASS_INFO_ISO },
        .pipelines = { PIPELINE_INFO_MAIN_DRAW,
                       PIPELINE_INFO_ISO_DRAW,
                       PIPELINE_INFO_MAIN_COLLIDER },
        .vertex_buffers = { VERTEX_BUFFER_INFO_MAIN_DRAW,
                            VERTEX_BUFFER_INFO_ISO_DRAW,
                            VERTEX_BUFFER_INFO_MAIN_COLLIDER }
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
    engine.add_system(std::make_unique<ecs::nj2DPhysicsSystem>());
    vulkan::RenderInfos render_queue{ mesh_registry,
                                      texture_registry,
                                      resources,
                                      render_buffer };

    mnt::TownLevel town{ engine };
    town.load();
    while (should_run) {
        engine.update();
        render_queue.update();
        renderer.draw_frame(render_queue);
        // return 0;
    }
}
