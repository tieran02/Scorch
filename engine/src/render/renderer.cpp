#include "pch.h"
#include "render/renderer.h"
#include "vk/vulkanRenderer.h"

using namespace SC;

namespace
{
	std::unique_ptr<Texture> gWhiteTexture;
}


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

uint8_t Renderer::FrameDataIndexCount() const
{
	switch (m_api)
	{
	case GraphicsAPI::VULKAN:
		return FRAME_OVERLAP_COUNT.at(to_underlying(GraphicsAPI::VULKAN));
	}

	return 1;
}

void Renderer::Init()
{
	if (!gWhiteTexture)
	{
		gWhiteTexture = Texture::Create(TextureType::TEXTURE2D, TextureUsage::COLOUR, Format::R8G8B8A8_SRGB);
		gWhiteTexture->Build(1, 1);

		std::array<uint8_t,4> pixel{ 255,255,255,255 };
		gWhiteTexture->CopyData(pixel.data(), pixel.size() * sizeof(uint8_t));
	}
}

void Renderer::Cleanup()
{
	gWhiteTexture.reset();
}

const SC::Texture* Renderer::WhiteTexture() const
{
	return gWhiteTexture.get();
}