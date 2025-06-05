#include "vulkan/Image.h"

#include <iostream>
#include <stdexcept>

#include <vulkan/util.h>

#include "vulkan/CommandPool.h"

namespace njin::vulkan {
    Image::Image(const LogicalDevice& device,
                 const PhysicalDevice& physical_device,
                 const VkImageCreateInfo& info) :
        device_{ &device },
        physical_device_{ &physical_device },
        info_{ info } {
        // make the image
        if (vkCreateImage(device_->get(), &info, nullptr, &image_) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create image");
        };
        format_ = info.format;

        // allocate memory for it
        VkMemoryRequirements memory_requirements{};
        vkGetImageMemoryRequirements(device_->get(),
                                     image_,
                                     &memory_requirements);

        VkMemoryAllocateInfo allocate_info{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext = nullptr,
            .allocationSize = memory_requirements.size,
            .memoryTypeIndex =
            find_memory_type(physical_device,
                             memory_requirements.memoryTypeBits,
                             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
        };
        if (vkAllocateMemory(device_->get(),
                             &allocate_info,
                             nullptr,
                             &memory_) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate memory");
        }

        // bind the allocated memory to the image object
        vkBindImageMemory(device_->get(), image_, memory_, 0);
    }

    Image::Image(const LogicalDevice& device, VkImage image, VkFormat format) :
        device_{ &device },
        format_{ format },
        image_{ image } {
        destroy_ = false;
    }

    Image::Image(Image&& other) noexcept :
        device_{ other.device_ },
        physical_device_{ other.physical_device_ },
        format_{ other.format_ },
        info_{ other.info_ },
        destroy_{ other.destroy_ } {
        if (image_ != other.image_ && image_ != VK_NULL_HANDLE) {
            // this image is being overwritten with another image
            clear();
        }
        memory_ = other.memory_;
        image_ = other.image_;

        other.device_ = nullptr;
        other.physical_device_ = nullptr;
        other.format_ = VK_FORMAT_UNDEFINED;
        other.image_ = VK_NULL_HANDLE;
        other.memory_ = VK_NULL_HANDLE;
        other.info_ = {};
        other.destroy_ = false;
    }

    Image& Image::operator=(Image&& other) noexcept {
        device_ = other.device_;
        physical_device_ = other.physical_device_;
        format_ = other.format_;

        if (image_ != other.image_ && image_ != VK_NULL_HANDLE) {
            // this image is being overwritten with another image
            clear();
        }

        image_ = other.image_;
        memory_ = other.memory_;
        info_ = other.info_;
        destroy_ = other.destroy_;

        other.device_ = nullptr;
        other.physical_device_ = nullptr;
        other.format_ = VK_FORMAT_UNDEFINED;
        other.image_ = VK_NULL_HANDLE;
        other.memory_ = VK_NULL_HANDLE;
        other.info_ = {};
        other.destroy_ = false;
        return *this;
    }

    Image::~Image() {
        if (device_ != VK_NULL_HANDLE && destroy_) {
            vkFreeMemory(device_->get(), memory_, nullptr);
            vkDestroyImage(device_->get(), image_, nullptr);
        }
    }

    VkImage Image::get() const {
        return image_;
    }

    VkFormat Image::get_format() const {
        return format_;
    }

    ImageWidth Image::get_width() const {
        return info_.extent.width;
    }

    ImageHeight Image::get_height() const {
        return info_.extent.height;
    }

    void Image::transition_layout(VkImageLayout new_layout) {
        const uint32_t graphics_index{
            physical_device_->get_graphics_family_index()
        };
        VkAccessFlags src_access_mask{};
        VkAccessFlags dst_access_mask{};
        VkPipelineStageFlags src_stage_mask{};
        VkPipelineStageFlags dst_stage_mask{};

        if (current_layout_ == VK_IMAGE_LAYOUT_UNDEFINED &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            src_access_mask = 0;
            dst_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            src_stage_mask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (current_layout_ == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            src_access_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
            src_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dst_stage_mask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("unsupported layout transition");
        }

        VkImageMemoryBarrier barrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = src_access_mask,
            .dstAccessMask = dst_access_mask,
            .oldLayout = current_layout_,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = graphics_index,
            .dstQueueFamilyIndex = graphics_index,
            .image = image_,
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel = 0,
                                  .levelCount = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1 }
        };
        CommandPool pool{ *device_, graphics_index };
        CommandBuffer command_buffer{
            pool.allocate_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        };
        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        vkCmdPipelineBarrier(command_buffer.get(),
                             src_stage_mask,
                             dst_stage_mask,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
        CommandBufferSubmitInfo end_info{ .should_wait_idle = true };
        command_buffer.end();
        command_buffer.submit(end_info);
        current_layout_ = new_layout;
    }

    void Image::load_buffer(const Buffer& buffer) {
        const uint32_t graphics_index{
            physical_device_->get_graphics_family_index()
        };
        CommandPool pool{ *device_, graphics_index };
        CommandBuffer command_buffer{
            pool.allocate_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
        };
        VkBufferImageCopy region{
            .bufferOffset = 0,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .mipLevel = 0,
                                  .baseArrayLayer = 0,
                                  .layerCount = 1 },
            .imageOffset = { 0, 0, 0 },
            .imageExtent = info_.extent
        };
        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        vkCmdCopyBufferToImage(command_buffer.get(),
                               buffer.get(),
                               image_,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               1,
                               &region);

        CommandBufferSubmitInfo end_info{ .should_wait_idle = true };
        command_buffer.end();
        command_buffer.submit(end_info);
    }

    void Image::clear() {
        vkFreeMemory(device_->get(), memory_, nullptr);
        vkDestroyImage(device_->get(), image_, nullptr);
        memory_ = VK_NULL_HANDLE;
        image_ = VK_NULL_HANDLE;
    }
}  // namespace njin::vulkan