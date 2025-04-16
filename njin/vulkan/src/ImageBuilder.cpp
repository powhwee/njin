#include "vulkan/ImageBuilder.h"

#include <stdexcept>

namespace njin::vulkan {
    ImageBuilder::ImageBuilder(const PhysicalDevice& physical_device,
                               const LogicalDevice& device) :
        device_{ &device },
        physical_device_{ &physical_device } {}

    Image ImageBuilder::build() {
        set_queue_family(physical_device_->get_graphics_family_index());
        if (!check_.all()) {
            throw std::runtime_error("Not all required properties were set");
        }
        return { *device_, *physical_device_, info_ };
    }

    void ImageBuilder::set_image_type(VkImageType type) {
        info_.imageType = type;
        check_.set(0, true);
    }

    void ImageBuilder::set_format(VkFormat format) {
        info_.format = format;
        check_.set(1, true);
    }

    void ImageBuilder::set_extent(VkExtent3D extent) {
        info_.extent = extent;
        check_.set(2, true);
    }

    void ImageBuilder::set_queue_family(uint32_t queue_family_index) {
        queue_family_indices_.push_back(queue_family_index);
        info_.queueFamilyIndexCount = queue_family_indices_.size();
        info_.pQueueFamilyIndices = queue_family_indices_.data();
        check_.set(3, true);
    }

    void ImageBuilder::set_initial_layout(VkImageLayout layout) {
        info_.initialLayout = layout;
        check_.set(4, true);
    }

    void ImageBuilder::set_usage(VkImageUsageFlags usage) {
        info_.usage = usage;
        check_.set(5, true);
    }
}  // namespace njin::vulkan
