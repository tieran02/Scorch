#pragma once
#include "shaderModule.h"

namespace SC
{
	enum class DescriptorBindingType
	{
		UNIFORM,
		COUNT
	};

	struct DescriptorBinding
	{
		DescriptorBindingType type;
		ShaderModuleFlags shaderStages;
	};

	struct DescriptorSetLayout
	{
	public:
		DescriptorSetLayout();
		DescriptorSetLayout(std::vector<DescriptorBinding>&& bindings);

		void AddBinding(DescriptorBindingType type, const ShaderModuleFlags&& stages);
	private:
		std::vector<DescriptorBinding> m_bindings;
	};

	class Buffer;
	class DescriptorSet
	{
	public:
		static std::unique_ptr<DescriptorSet> Create(const DescriptorSetLayout& layout);

		virtual void SetBuffer(const Buffer* buffer, uint32_t binding) = 0;
	protected:
		DescriptorSet(const DescriptorSetLayout& layout);
		DescriptorSetLayout m_layout;

	};
}