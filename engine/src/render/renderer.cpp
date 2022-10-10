#include "pch.h"
#include "render/renderer.h"
#include "vk/vulkanRenderer.h"

using namespace SC;



std::unique_ptr<Renderer> Renderer::Create(GraphicsAPI api)
{
	switch (api)
	{
	case GraphicsAPI::VULKAN:
		return std::make_unique<VulkanRenderer>();
	default:
		return nullptr;
	}
}

Renderer::Renderer(GraphicsAPI api) : m_api(api), m_currentFrame(0)
{

}

Renderer::~Renderer()
{

}

GraphicsAPI Renderer::GetApi() const
{
	return m_api;
}

uint8_t Renderer::FrameDataIndex() const
{
	switch (m_api)
	{
	case GraphicsAPI::VULKAN:
		return (m_currentFrame % FRAME_OVERLAP_COUNT.at(to_underlying(GraphicsAPI::VULKAN)));
	}

	return 1;
}
