#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan_core.h>

#include "DescriptorPool.h"
#include "DescriptorSetLayout.h"
#include "core/njTexture.h"
#include "vulkan/Buffer.h"
#include "vulkan/Image.h"
#include "vulkan/ImageView.h"
#include "vulkan/LogicalDevice.h"
#include "vulkan/Sampler.h"
#include "vulkan/config.h"

namespace njin::vulkan {
    class DescriptorSets {
        public:
        /**
        * Constructor
        * @param device Logical device
        * @param set_layout_infos List of descriptor set layout infos
        * @note All set layout infos provided to the constructor
        * will be instantiated. This means that actual buffers will be allocated,
        * memory bound, etc.
        */
        explicit DescriptorSets(const LogicalDevice& device,
                                const PhysicalDevice& physical_device,
                                const std::vector<SetLayoutInfo>&
                                set_layout_infos);

        VkDescriptorSetLayout
        get_descriptor_set_layout(const std::string& name) const;

        VkDescriptorSet get_descriptor_set(const std::string& name) const;

        /**
         * Replace data in a buffer descriptor binding's underlying resource
         * @tparam T Data type
         * @param set_name Name of descriptor set the binding belongs in
         * @param binding_name Name of binding
         * @param data New data
         */
        template<typename T>
        void write_descriptor_data(const std::string& set_name,
                                   const std::string& binding_name,
                                   const std::vector<T>& data);

        /**
         * Replace data in an image descriptor binding's underlying resources
         * @param set_name Name of descriptor set the binding belongs in
         * @param binding_name Name of binding
         * @param data New data
         */
        void
        write_descriptor_data(const std::string& set_name,
                              const std::string& binding_name,
                              const std::vector<const core::njTexture*>& data);

        private:
        /**
         * Write metadata to each binding in each descriptor set
         * @param set_layout_infos All SetLayoutInfos
         */
        void write_descriptor_sets(const std::vector<SetLayoutInfo>&
                                   set_layout_infos);

        /**
         * Create resources for all bindings in all set layouts
         * @param set_layout_infos Set layout infos
         * In each descriptor set layout there are several bindings,
         * and each binding has its own descriptor type. Each descriptor
         * type needs a unique kind of resource. For example, buffer descriptors
         * need to point to an underlying buffer. This method creates the
         * required resources for each binding.
         */
        void
        create_resources(const std::vector<SetLayoutInfo>& set_layout_infos);

        const LogicalDevice* device_{ nullptr };
        const PhysicalDevice* physical_device_{ nullptr };
        using SetName = std::string;
        using BindingName = std::string;
        using ImageSet = std::pair<Image, ImageView>;
        using Resource = std::variant<Buffer, std::vector<ImageSet>>;

        struct Hash {
            std::size_t
            operator()(const std::pair<SetName, BindingName>& key) const {
                std::hash<std::string> string_hasher{};
                return string_hasher(key.first) ^
                       (string_hasher(key.second) << 1);
            }
        };

        // Storages. Note that we don't care about the vector dynamically
        // resizing and changing addresses, because the wrapper classes
        // store the underlying handles, and those don't change.

        // sampler storage
        Sampler sampler_;
        DescriptorPool descriptor_pool_;

        std::unordered_map<SetName, DescriptorSetLayout> set_layouts_;
        std::unordered_map<SetName, VkDescriptorSet> descriptor_sets_;
        std::unordered_map<std::pair<SetName, BindingName>, uint32_t, Hash>
        binding_indices_;
        std::unordered_map<std::pair<SetName, BindingName>, Resource, Hash>
        resources_;
    };

    template<typename T>
    void DescriptorSets::write_descriptor_data(const std::string& set_name,
                                               const std::string& binding_name,
                                               const std::vector<T>& data) {
        Resource& resource{ resources_.at({ set_name, binding_name }) };
        // overloading into this method means that this resource should be a buffer
        Buffer& buffer{ std::get<Buffer>(resource) };
        buffer.load(data);
    }

}  // namespace njin::vulkan
