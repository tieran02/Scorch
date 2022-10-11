#include "pch.h"
#include "vk/vulkanDescriptorSet.h"
#include "../../include/scorch/core/app.h"
#include "../../include/scorch/vk/vulkanRenderer.h"

using namespace SC;


VulkanDescriptorSetLayout::VulkanDescriptorSetLayout() : DescriptorSetLayout(),
m_layout(VK_NULL_HANDLE)
{
	Init();
}

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(std::vector<DescriptorBinding>&& bindings) : DescriptorSetLayout(std::move(bindings)),
m_layout(VK_NULL_HANDLE)
{
	Init();
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
	m_deletionQueue.flush();
}

void VulkanDescriptorSetLayout::Init()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	std::vector<VkDescriptorSetLayoutBinding > vkSetBindings;

	for (int j = 0; j < m_bindings.size(); ++j)
	{
		VkDescriptorSetLayoutBinding binding = {};
		binding.binding = j;
		binding.descriptorCount = 1;

		switch (m_bindings[j].type)
		{
		case DescriptorBindingType::UNIFORM:
			binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		}

		if (m_bindings[j].shaderStages.test(to_underlying(ShaderStage::VERTEX)))
			binding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (m_bindings[j].shaderStages.test(to_underlying(ShaderStage::FRAGMENT)))
			binding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		vkSetBindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;

	//we are going to have 1 binding
	setinfo.bindingCount = m_bindings.size();
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = vkSetBindings.data();

	//create descriptor layout
	vkCreateDescriptorSetLayout(renderer->m_device, &setinfo, nullptr, &m_layout);
	CORE_ASSERT(m_layout, "Failed to create layout");

	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkDestroyDescriptorSetLayout(renderer->m_device, m_layout, nullptr);
		});
}

VulkanDescriptorSet::VulkanDescriptorSet(const DescriptorSetLayout* layout) : DescriptorSet(layout), 
m_descriptorSet(VK_NULL_HANDLE)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = renderer->m_descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &static_cast<const VulkanDescriptorSetLayout*>(layout)->m_layout;

	CORE_ASSERT(renderer->m_descriptorPool, "Descriptor cant be null");

	vkAllocateDescriptorSets(renderer->m_device, &allocInfo, &m_descriptorSet);
}

void VulkanDescriptorSet::SetBuffer(const Buffer* buffer, uint32_t binding)
{
	throw std::logic_error("The method or operation is not implemented.");
}