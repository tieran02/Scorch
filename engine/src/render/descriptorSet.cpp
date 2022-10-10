#include "pch.h"
#include "render/descriptorSet.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanDescriptorSet.h"

using namespace SC;

std::unique_ptr<DescriptorSet> DescriptorSet::Create(const DescriptorSetLayout& layout)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<VulkanDescriptorSet> descriptorSet{ nullptr };
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		descriptorSet = std::make_unique<VulkanDescriptorSet>(layout);
	}

	CORE_ASSERT(descriptorSet, "failed to create descriptorSet");
	return std::move(descriptorSet);
}

DescriptorSet::DescriptorSet(const DescriptorSetLayout& layout) : m_layout(layout)
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

