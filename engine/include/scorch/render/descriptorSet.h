#pragma once
#include "shaderModule.h"

namespace SC
{
	enum class DescriptorBindingType
	{
		UNIFORM,
		SAMPLER,
		COUNT
	};

	struct DescriptorBinding
	{
		DescriptorBindingType type;
		ShaderModuleFlags shaderStages;
	};

	class DescriptorSetLayout
	{
	public:
		static std::unique_ptr<DescriptorSetLayout> Create();
		static std::unique_ptr<DescriptorSetLayout> Create(std::vector<DescriptorBinding>&& bindings);
		virtual ~DescriptorSetLayout();

		void AddBinding(DescriptorBindingType type, const ShaderModuleFlags&& stages);
		const std::vector<DescriptorBinding>& Bindings() const;
	protected:
		DescriptorSetLayout();
		DescriptorSetLayout(std::vector<DescriptorBinding>&& bindings);

		std::vector<DescriptorBinding> m_bindings;
	};

	class Buffer;
	struct Texture;
	class DescriptorSet
	{
	public:
		static std::unique_ptr<DescriptorSet> Create(const DescriptorSetLayout* layout);

		virtual void SetBuffer(const Buffer* buffer, uint32_t binding) = 0;
		virtual void SetTexture(const Texture* texture, uint32_t binding) = 0;
		const DescriptorSetLayout* Layout() const;
	protected:
		DescriptorSet(const DescriptorSetLayout* layout);
		const DescriptorSetLayout* m_layout;

	};
}