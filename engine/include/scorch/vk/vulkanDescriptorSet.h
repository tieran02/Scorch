#pragma once
#include "render/descriptorSet.h"
#include <volk.h>
#include "core/utils.h"

namespace SC
{
	class VulkanDescriptorSetLayout : public DescriptorSetLayout
	{
	public:
		VulkanDescriptorSetLayout();
		VulkanDescriptorSetLayout(std::vector<DescriptorBinding>&& bindings);
		~VulkanDescriptorSetLayout();

		VkDescriptorSetLayout m_layout;
	private:
		void Init();

		DeletionQueue m_deletionQueue;
	};

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(const DescriptorSetLayout* layout);

		void SetBuffer(const Buffer* buffer, uint32_t binding) override;

		VkDescriptorSet m_descriptorSet;
	private:
		DeletionQueue m_deletionQueue;
	};
}