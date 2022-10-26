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

		int GetSamplerCount() const;

		VkDescriptorSetLayout m_layout;
	private:
		void Init();

		DeletionQueue m_deletionQueue;
	};

	class VulkanDescriptorSet : public DescriptorSet
	{
	public:
		VulkanDescriptorSet(const DescriptorSetLayout* layout);
		~VulkanDescriptorSet();

		void SetBuffer(const Buffer* buffer, uint32_t binding) override;
		void SetTexture(const Texture* texture, uint32_t binding) override;

		VkDescriptorSet m_descriptorSet;
	private:
		void CreateSamplers();

		std::vector<VkSampler> m_samplers; //Hold the vk samplers in the descriptor set (might want to have a separate sampler struct later for setting filter modes)
		DeletionQueue m_deletionQueue;
	};
}