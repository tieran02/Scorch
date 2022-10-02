#pragma once
#include "render/pipeline.h"

namespace SC
{
	enum class TextureType
	{
		TEXTURE2D
	};

	enum class TextureUsage
	{
		DEPTH,
	};

	struct Texture
	{
	public:
		static std::unique_ptr<Texture> Create(TextureType type, TextureUsage usage, Format format);
		virtual ~Texture();

		virtual bool Build(uint32_t width, uint32_t height) = 0;
	protected:
		Texture(TextureType type, TextureUsage usage, Format format);
		TextureType m_type;
		TextureUsage m_usage;
		Format m_format;
		uint32_t m_width, m_height;
	};
}