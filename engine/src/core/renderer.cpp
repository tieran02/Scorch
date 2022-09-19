#include "pch.h"
#include "core/renderer.h"
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

Renderer::~Renderer()
{

}
