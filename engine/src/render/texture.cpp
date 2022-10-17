#include "pch.h"
#include "render/texture.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanTexture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


using namespace SC;

size_t ImageData::Size() const
{
	return width * height * channels;
}


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

bool Texture::ReadImageFromFile(const std::string& path, ImageData& imageData)
{
	stbi_uc* rawPixelData = stbi_load(path.c_str(), &imageData.width, &imageData.height, &imageData.channels, STBI_rgb_alpha);
	imageData.channels = 4; //Set to 4 channels as were using STBI_rgb_alpha
	if (!rawPixelData)
	{
		CORE_ASSERT(false, string_format("Failed to load image: %s", path.c_str()));
		return false;
	}

	imageData.pixels.resize(imageData.Size());
	memmove(imageData.pixels.data(), rawPixelData, imageData.Size());

	return true;
}

