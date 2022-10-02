#include "pch.h"
#include "render/texture.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanTexture.h"

using namespace SC;

std::unique_ptr<Texture> Texture::Create(TextureType type, TextureUsage usage, Format format)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<Texture> texture{ nullptr };
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		texture = std::unique_ptr<Texture>(new VulkanTexture(type, usage, format));
	}

	CORE_ASSERT(texture, "failed to create texture");
	return std::move(texture);
}

Texture::Texture(TextureType type, TextureUsage usage, Format format) :
	m_type(type),
	m_usage(usage),
	m_format(format),
	m_width(0),
	m_height(0)
{

}

Texture::~Texture()
{

}
