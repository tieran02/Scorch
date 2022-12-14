#include "pch.h"
#include "vk/vulkanDescriptorSet.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanBuffer.h"
#include "vk/vulkanInitialiser.h"

using namespace SC;

namespace
{
	VkDescriptorType convertType(DescriptorBindingType type)
	{
		switch (type)
		{
		case DescriptorBindingType::UNIFORM:
			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		case DescriptorBindingType::SAMPLER:
			return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		}

		CORE_ASSERT(false, "Type not supported");
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}


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

		binding.descriptorType = convertType(m_bindings[j].type);

		if (m_bindings[j].shaderStages.test(ShaderStage::VERTEX))
			binding.stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (m_bindings[j].shaderStages.test(ShaderStage::FRAGMENT))
			binding.stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		vkSetBindings.push_back(binding);
	}

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;

	//we are going to have 1 binding
	setinfo.bindingCount = static_cast<uint32_t>(m_bindings.size());
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

int VulkanDescriptorSetLayout::GetSamplerCount() const
{
	int sampler2DCount = 0;
	for (int j = 0; j < m_bindings.size(); ++j)
	{
		if (m_bindings[j].type == DescriptorBindingType::SAMPLER)
			sampler2DCount++;
	}
	return sampler2DCount;
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

	VK_CHECK(vkAllocateDescriptorSets(renderer->m_device, &allocInfo, &m_descriptorSet));
	CORE_ASSERT(m_descriptorSet, "Failed to create descriptor set");

	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkFreeDescriptorSets(renderer->m_device, renderer->m_descriptorPool, 1, &m_descriptorSet);
		});

	CreateSamplers();
}

void VulkanDescriptorSet::SetBuffer(const Buffer* buffer, uint32_t binding)
{
	CORE_ASSERT(buffer, "Buffer can't be null");
	CORE_ASSERT(binding >= 0 && binding < m_layout->Bindings().size(), "binding index out of range");

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	//information about the buffer we want to point at in the descriptor
	VkDescriptorBufferInfo binfo;
	//it will be the camera buffer
	binfo.buffer = *static_cast<const VulkanBuffer*>(buffer)->GetBuffer();
	//at 0 offset
	binfo.offset = 0;
	//of the size of a camera data struct
	binfo.range = buffer->GetSize();

	VkWriteDescriptorSet setWrite = {};
	setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	setWrite.pNext = nullptr;

	//we are going to write into binding number 0
	setWrite.dstBinding = binding;
	//of the global descriptor
	setWrite.dstSet = m_descriptorSet;

	setWrite.descriptorCount = 1;
	//and the type is uniform buffer
	setWrite.descriptorType = convertType(m_layout->Bindings()[binding].type);;
	setWrite.pBufferInfo = &binfo;

	vkUpdateDescriptorSets(renderer->m_device, 1, &setWrite, 0, nullptr);
}

void VulkanDescriptorSet::SetTexture(const Texture* texture, uint32_t binding)
{
	CORE_ASSERT(texture, "Buffer can't be null");
	CORE_ASSERT(binding >= 0 && binding < m_layout->Bindings().size(), "binding index out of range");

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	//write to the descriptor set so that it points to our empire_diffuse texture
	VkDescriptorImageInfo imageBufferInfo;
	imageBufferInfo.sampler = m_samplers[binding];
	imageBufferInfo.imageView = static_cast<const VulkanTexture*>(texture)->m_imageView;
	imageBufferInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet texture1 = vkinit::WriteDescriptorImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_descriptorSet, &imageBufferInfo, binding);

	vkUpdateDescriptorSets(renderer->m_device, 1, &texture1, 0, nullptr);
}

void VulkanDescriptorSet::CreateSamplers()
{
	m_samplers.resize(static_cast<const VulkanDescriptorSetLayout*>(m_layout)->GetSamplerCount());
	if (m_samplers.empty()) return;

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	for (int i = 0; i < m_samplers.size(); ++i)
	{
		VkSamplerCreateInfo samplerInfo = vkinit::SamplerCreateInfo(VK_FILTER_LINEAR);
		vkCreateSampler(renderer->m_device, &samplerInfo, nullptr, &m_samplers[i]);

		m_deletionQueue.push_function([=]() {
			renderer->WaitOnFences();
			vkDestroySampler(renderer->m_device, m_samplers[i], nullptr);
			});
	}
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
	m_deletionQueue.flush();
}
