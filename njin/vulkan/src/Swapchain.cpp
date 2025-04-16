#include "vulkan/Swapchain.h"

#include <iostream>
#include <ostream>
#include <stdexcept>

#include <vulkan/ImageView.h>
#include <vulkan/Query.h>
#include <vulkan/util.h>

namespace {
    using namespace njin::vulkan;

    VkPresentModeKHR
    choose_swap_present_mode(const std::vector<VkPresentModeKHR>&
                             available_present_modes) {
        for (const auto& available_present_mode : available_present_modes) {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return available_present_mode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }
}  // namespace

namespace njin::vulkan {
    Swapchain::Swapchain(const LogicalDevice& device,
                         const PhysicalDevice& physical_device,
                         const Surface& surface) :
        device_{ device.get() } {
        // make queries to the surface to see what it supports, then make
        // our swapchain decisions from there

        // capabilities
        VkSurfaceCapabilitiesKHR capabilities{
            get_surface_capabilities(physical_device, surface)
        };
        extent_ = capabilities.currentExtent;

        // format support
        std::vector<VkSurfaceFormatKHR> formats{
            get_surface_formats(physical_device, surface)
        };
        // select a surface format to be used for images in this swapchain
        VkSurfaceFormatKHR surface_format{ choose_surface_format(formats) };
        image_format_ = surface_format.format;

        // presentation mode support
        std::vector<VkPresentModeKHR> present_modes{
            get_surface_presentation_modes(physical_device, surface)
        };
        VkPresentModeKHR present_mode{
            choose_swap_present_mode(present_modes)
        };

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.pNext = NULL;
        info.flags = 0;
        info.surface = surface.get();

        // double buffer at minimum
        //info.minImageCount = capabilities.minImageCount + 1;
        info.minImageCount = 2;

        // use the image format our selected surface format specifies
        info.imageFormat = surface_format.format;
        info.imageColorSpace = surface_format.colorSpace;
        info.imageExtent = capabilities.currentExtent;
        info.imageArrayLayers = 1;  // non-stereoscopic-3D app -> 1 as per spec
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        // swapchain images need to be shared between the graphics and present
        // queue if they are from different families, since the graphics queue
        // is the one writing to the image and the present queue is the one
        // presenting
        QueueFamilyIndices indices{ physical_device.get_indices() };
        if (indices.graphics_family != indices.present_family) {
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            std::vector<uint32_t> queue_family_indices{
                indices.graphics_family.value(),
                indices.present_family.value()
            };
            info.pQueueFamilyIndices = queue_family_indices.data();
        } else {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }
        info.preTransform = capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = present_mode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = VK_NULL_HANDLE;

        // make the swapchain
        if (vkCreateSwapchainKHR(device.get(), &info, nullptr, &swapchain_)) {
            throw std::runtime_error("failed to create swapchain");
        }

        // make an Image object referencing each image in the swapchain
        uint32_t image_count{ 0 };
        vkGetSwapchainImagesKHR(device.get(),
                                swapchain_,
                                &image_count,
                                nullptr);

        std::vector<VkImage> images(image_count);
        images.resize(image_count);
        vkGetSwapchainImagesKHR(device.get(),
                                swapchain_,
                                &image_count,
                                images.data());

        for (auto vulkan_image : images) {
            Image image{ device, vulkan_image, image_format_ };
            images_.push_back(std::move(image));
        }

        // make an image view for each image
        for (const auto& image : images_) {
            ImageView image_view{ device,
                                  image,
                                  extent_,
                                  VK_IMAGE_ASPECT_COLOR_BIT };
            image_views_.push_back(std::move(image_view));
        }
    }
}  // namespace njin::vulkan

Swapchain::~Swapchain() {
    vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}

VkSwapchainKHR Swapchain::get() const {
    return swapchain_;
}

VkSwapchainKHR* Swapchain::get_handle_address() {
    return &swapchain_;
}

VkFormat Swapchain::get_image_format() const {
    return image_format_;
}

VkExtent2D Swapchain::get_extent() const {
    return extent_;
}

std::vector<ImageView*> Swapchain::get_image_views() {
    std::vector<ImageView*> views{};
    for (auto& image_view : image_views_) {
        views.push_back(&image_view);
    }
    return views;
}
