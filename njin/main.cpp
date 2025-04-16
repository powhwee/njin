#include <chrono>
#include <ranges>
#include <thread>

#include "core/RenderQueue.h"
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
#include "vulkan/CommandBuffer.h"
#include "vulkan/DescriptorPoolBuilder.h"
#include "vulkan/DescriptorSetLayoutBuilder.h"
#include "vulkan/GraphicsPipelineBuilder.h"
#include "vulkan/ImageBuilder.h"
#include "vulkan/PhysicalDevice.h"
#include "vulkan/PipelineLayoutBuilder.h"
#include "vulkan/RenderPassBuilder.h"
#include "vulkan/Renderer.h"
#include "vulkan/SamplerBuilder.h"
#include "vulkan/ShaderModule.h"
#include "vulkan/Window.h"
#include "vulkan/pipeline_setup.h"
#include "vulkan/util.h"

#include <algorithm>

using namespace njin::vulkan;
using namespace njin;

constexpr int MAX_LIGHTS = 10;
constexpr int MAX_OBJECTS = 100;

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

    // prepare meshes from .meshes file
    core::njRegistry<core::njMesh> mesh_registry{};
    load_meshes("main.meshes", mesh_registry);
    std::vector<std::pair<std::string, const core::njMesh*>> meshes{};
    for (auto [name, mesh] : mesh_registry.get_map()) {
        meshes.push_back({ name, mesh });
    }

    BufferSettings vertex_buffer_settings{
        .size = calculate_meshes_size(mesh_registry),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };
    Buffer vertex_buffer{ logical_device,
                          physical_device,
                          vertex_buffer_settings };
    load_into_vertex_buffer(meshes, vertex_buffer);

    BufferSettings debug_buffer_settings{
        .size = sizeof(core::njVertex) * 2,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    Buffer debug_buffer{ logical_device,
                         physical_device,
                         debug_buffer_settings };
    math::njVec3f v0{ 0.f, 0.f, 0.f };
    math::njVec3f v1{ -100.f, 100.f, -100.f };
    math::njVec3f v2{ 0.f, 20.f, 0.f };
    // std::vector<core::njVertex> line_vertices{ v0, v1 };
    // debug_buffer.load(line_vertices);

    MeshIndices mesh_indices{ make_mesh_indices(meshes) };

    // prepare textures from .textures file
    core::njRegistry<core::njTexture> texture_registry{};
    load_textures("main.textures", texture_registry);

    std::vector<std::pair<std::string, Image>> texture_images{
        make_texture_images(physical_device, logical_device, texture_registry)
    };
    std::vector<ImageView> image_views{ make_image_views(logical_device,
                                                         texture_images) };
    TextureIndices texture_indices{ make_texture_indices(texture_images) };

    SamplerBuilder sampler_builder{ logical_device };
    Sampler sampler{ sampler_builder.build() };

    /** make the descriptor sets */
    // make the set layouts
    DescriptorSetLayoutBuilder set_layout_builder{ &logical_device };

    DescriptorSetLayout set_0_layout{ make_set_0_layout(set_layout_builder) };
    DescriptorSetLayout set_1_layout{ make_set_1_layout(set_layout_builder) };
    DescriptorSetLayout set_2_layout{
        make_set_2_layout(set_layout_builder, image_views, sampler)
    };

    // build a descriptor pool that can support all the set layouts we want
    DescriptorPoolBuilder pool_builder{ logical_device, physical_device };
    pool_builder.add_descriptor_set_layout(set_0_layout);
    pool_builder.add_descriptor_set_layout(set_1_layout);
    pool_builder.add_descriptor_set_layout(set_2_layout);

    DescriptorPool descriptor_pool{ pool_builder.build() };

    // allocate the descriptor sets
    DescriptorSet set_0{
        descriptor_pool.allocate_descriptor_set(set_0_layout)
    };
    DescriptorSet set_1{
        descriptor_pool.allocate_descriptor_set(set_1_layout)
    };
    DescriptorSet set_2{
        descriptor_pool.allocate_descriptor_set(set_2_layout)
    };

    // store in one array
    std::vector<DescriptorSet> descriptor_sets{};
    descriptor_sets.push_back(std::move(set_0));
    descriptor_sets.push_back(std::move(set_1));
    descriptor_sets.push_back(std::move(set_2));

    /** pipeline layout */
    PipelineLayoutBuilder pipeline_layout_builder{ logical_device };

    VkPushConstantRange model_index{ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                     .offset = 0,
                                     .size = sizeof(uint32_t) };

    VkPushConstantRange texture_index{
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = sizeof(uint32_t),  // the first uint32_t was the model index
        .size = sizeof(uint32_t)
    };

    pipeline_layout_builder.add_push_constant_range(model_index);
    pipeline_layout_builder.add_push_constant_range(texture_index);

    pipeline_layout_builder.add_descriptor_set_layout(set_0_layout);
    pipeline_layout_builder.add_descriptor_set_layout(set_1_layout);
    pipeline_layout_builder.add_descriptor_set_layout(set_2_layout);

    PipelineLayout pipeline_layout{ pipeline_layout_builder.build() };

    /** render pass */
    RenderPassBuilder render_pass_builder(logical_device);

    render_pass_builder.add_attachment_description(
    "color",
    { .flags = 0,
      .format = swapchain.get_image_format(),
      .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .store_op = VK_ATTACHMENT_STORE_OP_STORE,
      .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      .final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR });

    AttachmentReference color_attachment_reference{
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT
    };

    // depth attachment
    render_pass_builder.add_attachment_description(
    "depth",
    { .flags = 0,
      .format = VK_FORMAT_D32_SFLOAT,
      .load_op = VK_ATTACHMENT_LOAD_OP_CLEAR,
      .store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .stencil_load_op = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
      .stencil_store_op = VK_ATTACHMENT_STORE_OP_DONT_CARE,
      .initial_layout = VK_IMAGE_LAYOUT_UNDEFINED,
      .final_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });

    AttachmentReference depth_attachment_reference{
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT
    };

    std::vector<std::pair<AttachmentId, AttachmentReference>> color_attachments{
        { "color", color_attachment_reference }
    };
    std::pair<AttachmentId, AttachmentReference> depth_attachment{
        "depth",
        depth_attachment_reference
    };

    render_pass_builder
    .add_subpass_description("zero",
                             { .flags = 0,
                               .input_attachments = {},
                               .color_attachments = color_attachments,
                               .depth_attachment = depth_attachment });

    RenderPass render_pass{ render_pass_builder.build() };

    /** main graphics pipeline */
    GraphicsPipelineBuilder pipeline_builder{ logical_device,
                                              render_pass,
                                              pipeline_layout };

    /** shader modules */
    ShaderModule vertex_shader_module{ logical_device,
                                       SHADER_DIR "/shader.vert",
                                       VK_SHADER_STAGE_VERTEX_BIT };
    ShaderModule fragment_shader_module{ logical_device,
                                         SHADER_DIR "/shader.frag",
                                         VK_SHADER_STAGE_FRAGMENT_BIT };
    pipeline_builder.add_shader_stage(vertex_shader_module);
    pipeline_builder.add_shader_stage(fragment_shader_module);

    /** vertex input */
    pipeline_builder.add_vertex_input_binding_description(
    core::njVertex::get_binding_description());

    std::vector<VkVertexInputAttributeDescription> attribute_descriptions{
        core::njVertex::get_attribute_descriptions()
    };

    for (const auto& description : attribute_descriptions) {
        pipeline_builder.add_vertex_attribute_description(description);
    }

    pipeline_builder
    .set_primitive_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    GraphicsPipelineBuilder::RasterizationSettings rasterization_settings{
        .polygon_mode = VK_POLYGON_MODE_FILL,
        .cull_mode = VK_CULL_MODE_NONE,
        .front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    };

    pipeline_builder.set_rasterization_state(rasterization_settings);

    std::vector<VkDynamicState> dynamic_states{ VK_DYNAMIC_STATE_VIEWPORT,
                                                VK_DYNAMIC_STATE_SCISSOR };
    pipeline_builder.set_dynamic_state(dynamic_states);

    GraphicsPipeline main_pipeline{ pipeline_builder.build() };

    // debug pipeline
    GraphicsPipeline debug_pipeline{
        make_debug_pipeline(logical_device, render_pass, pipeline_layout)
    };

    core::RenderBuffer render_buffer{};

    core::RenderQueue render_queue{ mesh_indices,
                                    texture_indices,
                                    render_buffer };

    Renderer renderer{ physical_device, logical_device, swapchain,
                       render_pass,     main_pipeline,  debug_pipeline,
                       descriptor_sets, vertex_buffer,  debug_buffer };

    // initialize engine and add all the systems we want
    ecs::njEngine engine{};

    bool should_run{ true };
    engine.add_system(std::make_unique<ecs::njInputSystem>(should_run));
    engine.add_system(std::make_unique<ecs::njMovementSystem>());
    engine.add_system(std::make_unique<ecs::njRenderSystem>(render_buffer));
    engine.add_system(std::make_unique<ecs::njPhysicsSystem>());
    // engine.add_system(std::make_unique<ecs::njSceneGraphSystem>());

    // player
    ecs::njInputComponent input{};
    ecs::njTransformComponent transform{
        ecs::njTransformComponent::make(0, 10, 0)
    };
    ecs::njMovementIntentComponent intent{};
    ecs::njCollider player_collider{
        .transform = { math::njMat4Type::Translation, { 0.f, 0.f, 0.f } },
        .x_width = 2.f,
        .y_width = 2.f,
        .z_width = 2.f
    };
    ecs::njPhysicsComponent physics{ .velocity = { 0.f, 0.f, 0.f },
                                     .force = { 0, -10, 0 },
                                     .mass = 1,
                                     .collider = player_collider,
                                     .type = ecs::RigidBodyType::Dynamic };
    ecs::njPlayerArchetypeCreateInfo player_info{
        .name = "player",
        .transform = transform,
        .input = input,
        .mesh = { .mesh = "monkey", .texture = "pusheen" },
        .intent = intent,
        .physics = physics,
    };

    math::njMat4f cam_transform{ math::njMat4Type::Translation,
                                 { 20, 20, 20 } };
    ecs::njPlayerArchetype player{ player_info };
    ecs::EntityId player_id{ engine.add_archetype(player) };

    // camera
    ecs::njCameraArchetypeCreateInfo camera_info{
        .name = "camera",
        .transform = { cam_transform },
        .camera = { .up = { 0, 1, 0 },
                    .look_at = { 0, 0, 0 },
                    .far = 1000,
                    .near = 1,
                    .horizontal_fov = 60,
                    .width = 1920,
                    .height = 1080 }
    };

    ecs::njCameraArchetype camera{ camera_info };

    ecs::EntityId camera_id{ engine.add_archetype(camera) };

    // mnt::RoomBuilder builder{ 8, { 0, 0, 0 }, *mesh_registry.get("cube") };
    // auto tiles{ builder.build() };
    // for (const auto& tile : tiles) {
    //     engine.add_archetype(tile);
    // }

    ecs::njTransformComponent floor_transform{
        .transform = { math::njMat4Type::Translation, { 0.f, 0.f, 0.f } }
    };
    ecs::njObjectArchetypeCreateInfo floor_info{
        .name = "floor",
        .transform = floor_transform,
        .mesh = { .mesh = "flat_room", .texture = "pusheen" }
    };

    ecs::njObjectArchetype floor{ floor_info };
    ecs::EntityId floor_id{ engine.add_archetype(floor) };
    ecs::njCollider floor_collider{
        .transform = { math::njMat4Type::Translation, { 0.f, 0.f, 0.f } },
        .x_width = 20.f,
        .y_width = 2.f,
        .z_width = 2.f
    };
    ecs::njPhysicsComponent floor_physics{ .mass = 100.f,
                                           .collider = floor_collider,
                                           .type = ecs::RigidBodyType::Static };
    engine.add_component(floor_id, floor_physics);

    std::vector<DescriptorSet*> descriptor_set_handles{};
    for (auto& set : descriptor_sets) {
        descriptor_set_handles.push_back(&set);
    }
    while (should_run) {
        engine.update();

        render_queue.serialize(descriptor_set_handles);

        renderer.draw_frame(render_queue);
    }
}