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

Renderer::Renderer(GraphicsAPI api) : m_api(api)
{

}

Renderer::~Renderer()
{

}

GraphicsAPI Renderer::GetApi() const
{
	return m_api;
}

