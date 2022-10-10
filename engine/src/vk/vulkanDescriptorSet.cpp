#include "pch.h"
#include "vk/vulkanDescriptorSet.h"

using namespace SC;

VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSetLayout& layout) : DescriptorSet(layout)
{

}

void VulkanDescriptorSet::SetBuffer(const Buffer* buffer, uint32_t binding)
{
	throw std::logic_error("The method or operation is not implemented.");
}
