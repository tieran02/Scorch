#include "pch.h"
#include "render/pipeline.h"
#include "render/shaderModule.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanPipeline.h"
#include "render/descriptorSet.h"

using namespace SC;


uint32_t SC::ConvertFormatSize(Format format)
{
	switch (format)
	{

	case Format::UNDEFINED:
		return 0;
	case Format::D32_SFLOAT:
	case Format::R8_SRGB:
		return sizeof(char);
	case Format::R8G8_SRGB:
		return sizeof(char) * 2;
	case Format::R8G8B8_SRGB:
		return sizeof(char) * 3;
	case Format::R8G8B8A8_SRGB:
	case Format::B8G8R8A8_SRGB:
		return sizeof(char) * 4;
	case Format::R32_SFLOAT:
		return sizeof(float);
	case Format::R32G32_SFLOAT:
		return sizeof(float) * 2;
	case Format::R32G32B32_SFLOAT:
		return sizeof(float) * 3;
	case Format::R32G32B32A32_SFLOAT:
		return sizeof(float) * 4;
	}
	CORE_ASSERT(false, "Failed to find size for format")
	return 0;
}

VertexInputDescription::VertexInputDescription(VertexInputRate inputRate) :
	m_inputRate(inputRate)
{

} 
void VertexInputDescription::PushBackAttribute(Format&& format)
{
	m_attributes.emplace_back(format);
}

uint32_t VertexInputDescription::GetStride() const
{
	uint32_t totalSize = 0;
	for (int i = 0; i < m_attributes.size(); ++i)
	{
		totalSize += GetAttributeSize(i);
	}
	return totalSize;
}

uint32_t VertexInputDescription::GetAttributeSize(int index) const
{
	CORE_ASSERT(index >= 0 && index < m_attributes.size(), "index is invalid size");
	if (index >= 0 && index < m_attributes.size())
	{
		return ConvertFormatSize(m_attributes[index]);
	}
	return 0;
}

uint32_t VertexInputDescription::GetAttributeOffset(int index) const
{
	CORE_ASSERT(index >= 0 && index < m_attributes.size(), "index is invalid size");
	uint32_t offset = 0;
	for (int i = 0; i < index; ++i)
	{
		offset += GetAttributeSize(i);
	}
	return offset;
}

const std::vector<Format>& VertexInputDescription::Attributes() const
{
	return m_attributes;
}

VertexInputRate VertexInputDescription::InputRate() const
{
	return m_inputRate;
}

Viewport::Viewport() : 
	x{ 0 },
	y{ 0 },
	w{ 0 },
	h{ 0 },
	minDepth(0.0f),
	maxDepth(1.0f)
{

}

Viewport::Viewport(float x, float y, float w, float h) :
	x{ x },
	y{ y },
	w{ w },
	h{ h },
	minDepth(0.0f),
	maxDepth(1.0f)
{

}

Scissor::Scissor() :
	offsetX{ 0 },
	offsetY{ 0 },
	extentX{ 0 },
	extentY{ 0 }
{

}

Scissor::Scissor(uint32_t extentX, uint32_t extentY) :
	offsetX{ 0 },
	offsetY{ 0 },
	extentX{ extentX },
	extentY{ extentY }
{

}

std::unique_ptr<SC::PipelineLayout> PipelineLayout::Create()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<VulkanPipelineLayout> pipelineLayout{ nullptr };
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		pipelineLayout = std::unique_ptr<VulkanPipelineLayout>(new VulkanPipelineLayout());
	}

	CORE_ASSERT(pipelineLayout, "failed to create pipeline layout");
	return std::move(pipelineLayout);
}

PipelineLayout::PipelineLayout()
{

}

PipelineLayout::~PipelineLayout()
{

}

void PipelineLayout::AddPushConstant(ShaderModuleFlags stages, uint32_t size)
{
	m_pushConstants.emplace_back(stages, size);
}

const std::vector<SC::PushConstant>& PipelineLayout::PushConstants() const
{
	return m_pushConstants;
}

void PipelineLayout::AddDescriptorSetLayout(const DescriptorSetLayout* layout)
{
	m_descriptorSetLayouts.push_back(layout);
}

const std::vector<const DescriptorSetLayout*>& PipelineLayout::DescriptorSetLayouts() const
{
	return m_descriptorSetLayouts;
}

std::unique_ptr<Pipeline> Pipeline::Create(const ShaderModule& module)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;
	
	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<Pipeline> pipeline{nullptr};
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		pipeline = std::unique_ptr<VulkanPipeline>(new VulkanPipeline(module));
	}

	CORE_ASSERT(pipeline, "failed to create pipeline");
	return std::move(pipeline);
}

Pipeline::Pipeline(const ShaderModule& module) :
	shaderModule(&module),
	pipelineLayout(nullptr),
	primitiveTopolgy(PrimitiveTopolgy::TRIANGLE_LIST),
	polygonMode(PolygonMode::FILL)
{

	//Attempt to construct the viewport given the app window as the default param
	const App* app = App::Instance();
	if (app)
	{
		int windowWidth{ 0 }, windowHeight{ 0 };
		app->GetWindowExtent(windowWidth, windowHeight);

		viewport.w = static_cast<float>(windowWidth);
		viewport.h = static_cast<float>(windowHeight);
		scissor.extentX = static_cast<uint32_t>(windowWidth);
		scissor.extentY = static_cast<uint32_t>(windowHeight);
	}
	
}

Pipeline::~Pipeline()
{

}
