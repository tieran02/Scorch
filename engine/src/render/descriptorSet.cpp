#include "pch.h"
#include "render/descriptorSet.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanDescriptorSet.h"

using namespace SC;

std::unique_ptr<DescriptorSet> DescriptorSet::Create(const DescriptorSetLayout* layout)
{
	CORE_ASSERT(layout, "Layout can't be null");

	SCORCH_API_CREATE(DescriptorSet, layout);
}

DescriptorSet::DescriptorSet(const DescriptorSetLayout* layout) : m_layout(layout)
{

}

const DescriptorSetLayout* DescriptorSet::Layout() const
{
	return m_layout;
}

DescriptorSet::~DescriptorSet()
{

}

DescriptorSetLayout::DescriptorSetLayout()
{

}

DescriptorSetLayout::DescriptorSetLayout(std::vector<DescriptorBinding>&& bindings) :
	m_bindings(std::move(bindings))
{

}

void DescriptorSetLayout::AddBinding(DescriptorBindingType type, const ShaderModuleFlags&& stages)
{
	m_bindings.emplace_back(type, stages);
}

const std::vector<DescriptorBinding>& DescriptorSetLayout::Bindings() const
{
	return m_bindings;
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Create()
{
	SCORCH_API_CREATE(DescriptorSetLayout);
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Create(std::vector<DescriptorBinding>&& bindings)
{
	SCORCH_API_CREATE(DescriptorSetLayout, std::move(bindings));
}

DescriptorSetLayout::~DescriptorSetLayout()
{

}

