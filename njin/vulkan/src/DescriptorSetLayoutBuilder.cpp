#include "vulkan/DescriptorSetLayoutBuilder.h"

namespace njin::vulkan {
    DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(const LogicalDevice*
                                                           device) :
        device_{ device } {};

    void
    DescriptorSetLayoutBuilder::add_binding(const std::string& name,
                                            const DescriptorSetBindingSettings&
                                            settings) {
        // add the common binding settings
        const VkDescriptorSetLayoutBinding info{
            .binding = settings.binding,
            .descriptorType = settings.descriptor_type,
            .descriptorCount = settings.descriptor_count,
            .stageFlags = settings.stage_flags,
            .pImmutableSamplers = nullptr
        };
        set_bindings_.push_back(info);

        info_.bindingCount = set_bindings_.size();
        info_.pBindings = set_bindings_.data();

        // record the name associated with this binding
        binding_map_.insert({ name, settings });

        // also record how many descriptor "slots" this particular binding
        // will need
        const VkDescriptorPoolSize size{
            .type = settings.descriptor_type,
            .descriptorCount = settings.descriptor_count
        };
        pool_sizes_.push_back(size);
    }

    DescriptorSetLayout DescriptorSetLayoutBuilder::build() const {
        return { *device_, info_, binding_map_ };
    }

    void DescriptorSetLayoutBuilder::reset() {
        *this = DescriptorSetLayoutBuilder{ device_ };
    }

}  // namespace njin::vulkan