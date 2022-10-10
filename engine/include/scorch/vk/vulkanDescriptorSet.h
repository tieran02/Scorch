#pragma once
#include "render/descriptorSet.h"

namespace SC
{
	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(const DescriptorSetLayout& layout);

		void SetBuffer(const Buffer* buffer, uint32_t binding) override;

	};
}