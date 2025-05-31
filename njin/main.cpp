#include <chrono>
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

namespace {
    /**
     * Calculate the total size, in bytes, of all meshes in the mesh registry
     * @param mesh_registry Mesh registry to calculate total mesh sizes for
     * @return Total mesh size in bytes
     */
    size_t
    calculate_meshes_size(const core::njRegistry<core::njMesh>& mesh_registry) {
        const std::vector<const core::njMesh*> meshes{
            mesh_registry.get_records()
        };
        size_t total_size{ 0 };
        for (const core::njMesh* mesh : meshes) {
            total_size += mesh->get_size();
        }
        return total_size;
    }

    int calculate_total_vertex_count(const core::njRegistry<core::njMesh>&
                                     mesh_registry) {
        int count{ 0 };
        const std::vector<const core::njMesh*> meshes{
            mesh_registry.get_records()
        };
        for (const core::njMesh* mesh : meshes) {
            count += mesh->get_vertices().size();
        }

        return count;
    }

    /**
     * Create a map that associates the name of a mesh with its offset + length
     * in the vertex buffer
     * @param meshes Array of meshes loaded into the vertex buffer
     * @see load_into_vertex_buffer
     * @return Mesh index
     * @note offset refers to the **vertex** offset, not the byte offset.
     * That is, if model A and B both have 3 vertices, and each vertex is
     * 16B, then the offset of B is **not** 16 * 3, but just 3.
     */

    MeshIndices make_mesh_indices(
    const std::vector<std::pair<std::string, const core::njMesh*>>& meshes) {
        int current_offset{ 0 };
        MeshIndices indices{};
        for (auto [name, mesh] : meshes) {
            MeshInfo info{ current_offset, mesh->get_vertex_count() };
            indices.insert({ name, info });
            current_offset += mesh->get_vertex_count();
        }

        return indices;
    }

    /**
     * Load an array of meshes into the vertex buffer
     * @param pairs List of mesh names and meshes
     * @param buffer Vertex buffer
     */
    void load_into_vertex_buffer(
    const std::vector<std::pair<std::string, const core::njMesh*>>& pairs,
    Buffer& buffer) {
        // extract vertices into contiguous array
        std::vector<core::njVertex> vertices{};
        for (const auto mesh : pairs | std::views::values) {
            std::vector<core::njVertex> mesh_vertices{ mesh->get_vertices() };
            vertices.insert(vertices.end(),
                            mesh_vertices.begin(),
                            mesh_vertices.end());
        }

        // load into buffer
        buffer.load(vertices);
    }

    /**
     * Make descriptor set 0, that handles MVP matrices
     * @param builder Set layout builder
     * @return MVP descriptor set layout
     * @note The passed in builder may or not have been used before
     */
    DescriptorSetLayout make_set_0_layout(DescriptorSetLayoutBuilder& builder) {
        builder.reset();
        // binding 0: model matrices
        DescriptorSetBindingBufferSettings model_specific_settings{
            .descriptor_size = sizeof(math::njMat4f)
        };  // a single global transform matrix

        DescriptorSetBindingSettings model_binding_settings{
            .binding = 0,
            .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptor_count = MAX_OBJECTS,
            .stage_flags = VK_SHADER_STAGE_VERTEX_BIT,
            .extra_settings = model_specific_settings
        };
        builder.add_binding("model", model_binding_settings);

        // binding 1: view + projection matrix
        DescriptorSetBindingBufferSettings vp_specific_settings{
            .descriptor_size = sizeof(math::njMat4f) * 2,
        };

        DescriptorSetBindingSettings vp_binding_settings{
            .binding = 1,
            .descriptor_type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptor_count = 1,
            .stage_flags = VK_SHADER_STAGE_VERTEX_BIT,
            .extra_settings = vp_specific_settings
        };

        builder.add_binding("vp", vp_binding_settings);

        DescriptorSetLayout set_0_layout{ builder.build() };
        builder.reset();
        return set_0_layout;
    }

    /**
     * Make descriptor set 1, that handles lights
     * @param builder Set layout builder
     * @return Lights descriptor set layout
     * @note The passed in builder may or not have been used before
     */
    DescriptorSetLayout make_set_1_layout(DescriptorSetLayoutBuilder& builder) {
        builder.reset();
        DescriptorSetBindingBufferSettings lights_specific_settings{
            .descriptor_size = sizeof(math::njVec3f) + sizeof(math::njVec4f) + 4
        };  // position + color = 28
        // 28 + 4 = 32 -> fulfills 16B alignment requirement for ssbos

        DescriptorSetBindingSettings lights_binding_settings{
            .binding = 0,
            .descriptor_type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptor_count = MAX_LIGHTS,
            .stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .extra_settings = lights_specific_settings
        };
        builder.add_binding("lights", lights_binding_settings);
        DescriptorSetLayout set_1_layout{ builder.build() };
        builder.reset();
        return set_1_layout;
    }

    /**
     * Make descriptor set 2, which handles texture images
     * @param builder Descriptor set layout builder
     * @param image_views Array of image views to texture images
     * @param sampler Image sampler to be used
     * @return Textures descriptor set layout
     * @note The passed in builder may or not have been used before
     */
    DescriptorSetLayout
    make_set_2_layout(DescriptorSetLayoutBuilder& builder,
                      const std::vector<ImageView>& image_views,
                      const Sampler& sampler) {
        builder.reset();
        std::vector<VkImageView> vk_image_views{};
        for (const auto& image_view : image_views) {
            vk_image_views.push_back(image_view.get());
        }

        DescriptorSetBindingImageSetting image_settings{
            .sampler = sampler.get(),
            .image_views = vk_image_views,
            .image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };
        DescriptorSetBindingSettings texture_binding{
            .binding = 0,
            .descriptor_type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptor_count = static_cast<uint32_t>(image_views.size()),
            .stage_flags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .extra_settings = image_settings
        };
        builder.add_binding("texture", texture_binding);
        DescriptorSetLayout set_2_layout{ builder.build() };
        return set_2_layout;
    }

    /**
     * Create Images for all textures in a texture registry
     * @param physical_device Physical device
     * @param logical_device Logical device
     * @param texture_registry Texture registry
     * @return Array of allocated images
     */
    std::vector<std::pair<std::string, Image>>
    make_texture_images(const PhysicalDevice& physical_device,
                        const LogicalDevice& logical_device,
                        const core::njRegistry<core::njTexture>&
                        texture_registry) {
        std::vector<std::pair<std::string, Image>> texture_images{};
        for (auto [name, texture] : texture_registry.get_map()) {
            ImageBuilder builder{ physical_device, logical_device };
            builder.set_image_type(VK_IMAGE_TYPE_2D);
            builder.set_format(VK_FORMAT_R8G8B8A8_SRGB);
            builder.set_extent({ texture->get_width(),
                                 texture->get_height(),
                                 1 });
            builder.set_initial_layout(VK_IMAGE_LAYOUT_UNDEFINED);

            // transfer bit:
            // we will load the pixel data from texture into a staging buffer first,
            // then use vkCmdCopyBufferToImage to move the data into the VkImage
            // sampled bit: this image will be sampled by a VkSampler
            builder.set_usage(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                              VK_IMAGE_USAGE_SAMPLED_BIT);

            Image image{ builder.build() };

            // load pixel data into a staging buffer
            BufferSettings settings{
                .size = texture->get_size_uint64(),
                .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            };
            Buffer staging{ logical_device, physical_device, settings };
            staging.load(texture->get_data());

            // load the pixel data into the images
            image.transition_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            image.load_buffer(staging);
            image.transition_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            texture_images.push_back({ name, std::move(image) });
        }
        return texture_images;
    }

    /**
     * Associate texture names with their
     * @param images Array of texture names and their images
     * @return Texture index
     */
    TextureIndices
    make_texture_indices(const std::vector<std::pair<std::string, Image>>&
                         images) {
        TextureIndices indices{};
        int current_index{ 0 };
        for (const auto& name : images | std::views::keys) {
            indices[name] = current_index;
            ++current_index;
        }
        return indices;
    }

    /**
     * Create image views that reference each image in an array of Images
     * @param device
     * @param images Array of images
     * @return Array of image views
     */
    std::vector<ImageView>
    make_image_views(const LogicalDevice& device,
                     const std::vector<std::pair<std::string, Image>>& images) {
        std::vector<ImageView> image_views{};
        for (const auto& image : images | std::views::values) {
            ImageView image_view{ device,
                                  image,
                                  { image.get_width(), image.get_height() },
                                  VK_IMAGE_ASPECT_COLOR_BIT };

            image_views.push_back(std::move(image_view));
        }

        return image_views;
    }
}  // namespace

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
        .set_layouts = { DESCRIPTOR_SET_LAYOUT_INFO_MVP },
        .render_passes = { RENDER_PASS_INFO_MAIN },
        .pipelines = { PIPELINE_INFO_MAIN_DRAW },
        .vertex_buffers = { VERTEX_BUFFER_INFO_MAIN_DRAW }
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

    // initialize engine and add all the systems we want
    ecs::njEngine engine{};
    bool should_run{ true };
    engine.add_system(std::make_unique<ecs::njInputSystem>(should_run));
    engine.add_system(std::make_unique<ecs::njMovementSystem>());
    core::RenderBuffer render_buffer{};
    engine.add_system(std::make_unique<ecs::njRenderSystem>(render_buffer));

    ecs::njCameraArchetypeCreateInfo camera_info{
        .name = "camera",
        .transform = ecs::njTransformComponent::make(10.f, 20.f, 10.f),
        .camera = { .up = { 0.f, 1.f, 0.f },
                    .look_at = { 0.f, 0.f, 0.f },
                    .far = { 200.f },
                    .near = { 1.f },
                    .horizontal_fov = { 90.f },
                    .width = 1280,
                    .height = 720 }
    };
    ecs::njCameraArchetype camera_archetype{ camera_info };
    engine.add_archetype(camera_archetype);

    ecs::njObjectArchetypeCreateInfo object_info{
        .name = "cube",
        .transform = ecs::njTransformComponent::make(0.f, 0.f, 0.f),
        .mesh = "cube"
    };
    ecs::njObjectArchetype object_archetype{ object_info };
    engine.add_archetype(object_archetype);

    while (should_run) {
        engine.update();
        vulkan::RenderInfos render_queue{ mesh_registry,
                                          resources,
                                          render_buffer };
        renderer.draw_frame(render_queue);
        return 0;
    }
}