#pragma once
#include "render/texture.h"
#include <volk.h>
#include "core/utils.h"
#include "vk_mem_alloc.h"

namespace SC
{
	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(TextureType type, TextureUsage usage, Format format);
		~VulkanTexture();

		bool Build(uint32_t width, uint32_t height, bool generateMipmaps) override;
		bool LoadFromFile(const std::string& path) override;
		bool CopyData(const void* data, size_t size) override;
	public:
		VkImage m_image;
		VmaAllocation m_allocation;
		VkImageView  m_imageView;
	private:
		DeletionQueue m_deletionQueue;
		uint32_t m_mipLevels;
	};

	struct VulkanRenderTarget : RenderTarget
	{
	public:
		VulkanRenderTarget(Format format);
		~VulkanRenderTarget();

		bool Build(uint32_t width, uint32_t height) override;

		VkImage m_image;
		VmaAllocation m_allocation;
		VkImageView  m_imageView;
		VkFramebuffer m_framebuffer;
	private:
		DeletionQueue m_deletionQueue;
	};
}