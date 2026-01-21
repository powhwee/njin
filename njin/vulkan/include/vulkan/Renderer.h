#pragma once

#include <memory>
#include "Fence.h"
#include "Semaphore.h"
#include "vulkan/RenderResources.h"
#include "vulkan/SubpassModule.h"
#include "vulkan/config_classes.h"

namespace njin::vulkan {
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    class Renderer {
        public:
        explicit Renderer(const LogicalDevice& logical_device,
                          const PhysicalDevice& physical_device,
                          Swapchain& swapchain,
                          const RendererInfo& info,
                          RenderResources& resources);
        void draw_frame(RenderInfos& render_infos);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        private:
        const LogicalDevice* device_{ nullptr };
        Swapchain* swapchain_{ nullptr };
        std::vector<std::unique_ptr<Semaphore>> image_available_semaphores_;
        std::vector<std::unique_ptr<Semaphore>> render_finished_semaphores_;
        std::vector<std::unique_ptr<Fence>> in_flight_fences_;
        uint32_t current_frame_{ 0 };

        void run_render_pass(CommandBuffer& command_buffer,
                             const std::string& render_pass_name,
                             uint32_t image_index,
                             const RenderInfos& render_infos) const;
        // ordered list of render passes
        std::vector<std::string> render_passes_;
        std::unordered_map<std::string, std::vector<SubpassModule>>
        subpass_modules_;
        RenderResources* resources_{ nullptr };
    };
}  // namespace njin::vulkan


