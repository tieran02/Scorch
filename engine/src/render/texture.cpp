#include "pch.h"
#include "render/texture.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanTexture.h"

using namespace SC;

size_t ImageData::Size() const
{
	return width * height * channels;
}


std::unique_ptr<Texture> Texture::Create(TextureType type, TextureUsage usage, Format format)
{
	SCORCH_API_CREATE(Texture, type, usage, format);
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
	//stbi_uc* rawPixelData = stbi_load(path.c_str(), &imageData.width, &imageData.height, &imageData.channels, STBI_rgb_alpha);
	//imageData.channels = 4; //Set to 4 channels as were using STBI_rgb_alpha
	//if (!rawPixelData)
	//{
	//	CORE_ASSERT(false, string_format("Failed to load image: %s", path.c_str()));
	//	return false;
	//}

	//imageData.pixels.resize(imageData.Size());
	//memmove(imageData.pixels.data(), rawPixelData, imageData.Size());

	return true;
}

Format Texture::GetFormat() const
{
	return m_format;
}


std::unique_ptr<SC::RenderTarget> RenderTarget::Create(std::vector<Format>&& attachmentFormats, uint32_t width, uint32_t height)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<RenderTarget> renderTarget{ nullptr };
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		renderTarget = std::unique_ptr<RenderTarget>(new VulkanRenderTarget(std::move(attachmentFormats), width, height));
	}

	CORE_ASSERT(renderTarget, "failed to create renderTarget");
	return std::move(renderTarget);
}

RenderTarget::RenderTarget(std::vector<Format>&& attachmentFormats, uint32_t width, uint32_t height) :
	m_width(width),
	m_height(height),
	m_attachmentFormats(std::move(attachmentFormats))
{
	m_textures.resize(m_attachmentFormats.size());
}

RenderTarget::~RenderTarget()
{
	for (auto& texture : m_textures)
	{
		if (texture.second)
			delete texture.first;
	}
}

bool RenderTarget::BuildAttachmentTexture(uint32_t attachmentIndex)
{
	CORE_ASSERT(attachmentIndex >= 0 && attachmentIndex < m_attachmentFormats.size(), "attachmentIndex is out of bounds");
	if (attachmentIndex < 0 && attachmentIndex >= m_attachmentFormats.size()) return false;

	TextureUsage usage = m_attachmentFormats[attachmentIndex] == Format::D32_SFLOAT ? TextureUsage::DEPTH : TextureUsage::COLOUR;
	auto texture = Texture::Create(TextureType::TEXTURE2D, usage, m_attachmentFormats[attachmentIndex]);
	texture->Build(m_width, m_height, false);

	//Texture::Create returns a unique_ptr but m_textures can also hold texture that isn't owned by RenderTarget
	//So transfer ownership to a raw ptr and mark as having ownership (will be delete in ~RenderTarget())
	//Not very clean and probably should be refactored at a later date
	m_textures[attachmentIndex] = std::make_pair(texture.release(), true);

	return true;
}

bool RenderTarget::SetAttachmentTexture(uint32_t attachmentIndex, Texture* texture)
{
	CORE_ASSERT(attachmentIndex >= 0 && attachmentIndex < m_attachmentFormats.size(), "attachmentIndex is out of bounds");
	if (attachmentIndex < 0 && attachmentIndex >= m_attachmentFormats.size()) return false;

	m_textures[attachmentIndex] = std::make_pair(texture, false);

	return true;
}

const SC::Texture* RenderTarget::GetAttachmentTexture(uint32_t attachmentIndex)
{
	CORE_ASSERT(attachmentIndex >= 0 && attachmentIndex < m_attachmentFormats.size(), "attachmentIndex is out of bounds");
	if (attachmentIndex < 0 && attachmentIndex >= m_attachmentFormats.size()) return nullptr;

	return m_textures.at(attachmentIndex).first;
}
