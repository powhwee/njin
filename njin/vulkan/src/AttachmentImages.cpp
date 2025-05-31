#include "vulkan/AttachmentImages.h"

namespace njin::vulkan {
    AttachmentImages::AttachmentImages(const LogicalDevice& device,
                                       const PhysicalDevice& physical_device,
                                       Swapchain& swapchain,
                                       const std::vector<AttachmentImageInfo>&
                                       infos) :
        device_{ &device },
        swapchain_{ &swapchain } {
        // create the images
        for (const AttachmentImageInfo& attachment_image_info : infos) {
            VkImageCreateInfo info{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = attachment_image_info.flags,
                .imageType = attachment_image_info.image_type,
                .format = attachment_image_info.format,
                .extent = { .width = swapchain.get_extent().width,
                            .height = swapchain.get_extent().height,
                            .depth = 1 },
                .mipLevels = attachment_image_info.mip_levels,
                .arrayLayers = attachment_image_info.array_layers,
                .samples = attachment_image_info.samples,
                .tiling = attachment_image_info.tiling,
                .usage = attachment_image_info.usage,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,  // as per the spec
            };
            image_infos_[attachment_image_info.name] = info;

            Image image{ device, physical_device, info };

            images_.emplace(attachment_image_info.name, std::move(image));
        }

        // For swapchain images we create a "dummy" VkImageCreateInfo
        // Swapchain images are created by the swapchain instead of manually
        // by us, but the documentation does state the implicit parameters
        // the swapchain uses to create its images.
        // See https://docs.vulkan.org/spec/latest/chapters/VK_KHR_surface/wsi.html#swapchain-wsi-image-create-info
        VkExtent2D swapchain_extent{ swapchain.get_extent() };
        VkImageCreateInfo swapchain_image_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = swapchain.get_image_format(),
            .extent = { swapchain_extent.width, swapchain_extent.height, 1 },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 0,
            .pQueueFamilyIndices = nullptr,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        image_infos_["swapchain"] = swapchain_image_info;
    }

    VkImageCreateInfo
    AttachmentImages::get_image_create_info(const std::string& name) const {
        return image_infos_.at(name);
    }

    std::vector<std::vector<VkImageView>>
    AttachmentImages::make_image_view_sets(const std::vector<std::string>&
                                           names) {
        // get the image views from the swapchain first
        std::vector<ImageView*> swapchain_image_views{
            swapchain_->get_image_views()
        };

        size_t count{ swapchain_image_views.size() };

        std::vector<std::vector<VkImageView>> image_view_sets{};
        for (int i{ 0 }; i < count; ++i) {
            std::vector<VkImageView> image_view_set{};
            for (const std::string& name : names) {
                // retrieve the relevant image info
                VkImageCreateInfo image_info{ image_infos_.at(name) };

                VkImageAspectFlags aspect_mask{};
                if (image_info.usage == VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
                    aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
                } else if (image_info.usage ==
                           VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
                    if (image_info.format == VK_FORMAT_D32_SFLOAT) {
                        // depth only
                        aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
                    }
                }
                // If the swapchain image is referred to, we already have
                // the image view. Otherwise, create an image view of the
                // specified image
                VkImageView vk_image_view{};
                if (name == "swapchain") {
                    vk_image_view = swapchain_image_views[i]->get();
                } else {
                    ImageView image_view{ *device_,
                                          images_.at(name),
                                          swapchain_->get_extent(),
                                          aspect_mask };
                    vk_image_view = image_view.get();
                    image_views_.push_back(std::move(image_view));
                }
                image_view_set.push_back(vk_image_view);
            }
            image_view_sets.push_back(image_view_set);
        }

        return image_view_sets;
    }

    VkExtent2D AttachmentImages::get_extent() const {
        return swapchain_->get_extent();
    }

}  // namespace njin::vulkan