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

		bool Build(uint32_t width, uint32_t height) override;
		bool LoadFromFile(const std::string& path) override;
		bool CopyData(void* data, size_t size) override;
	public:
		VkImage m_image;
		VmaAllocation m_allocation;
		VkImageView  m_imageView;
	private:
		DeletionQueue m_deletionQueue;
	};
}