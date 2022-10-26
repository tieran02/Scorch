#pragma once
#include "render/pipeline.h"

namespace SC
{
	struct ImageData
	{
		std::vector<unsigned char> pixels;
		int width;
		int height;
		int channels;

		size_t Size() const;
	};

	enum class TextureType
	{
		TEXTURE2D
	};

	enum class TextureUsage
	{
		DEPTH,
		COLOUR,
	};

	struct Texture
	{
	public:
		static std::unique_ptr<Texture> Create(TextureType type, TextureUsage usage, Format format);
		virtual ~Texture();

		virtual bool Build(uint32_t width, uint32_t height) = 0;
		virtual bool LoadFromFile(const std::string& path) = 0;
	protected:
		bool ReadImageFromFile(const std::string& path, ImageData& imageData);

		Texture(TextureType type, TextureUsage usage, Format format);
		TextureType m_type;
		TextureUsage m_usage;
		Format m_format;
		uint32_t m_width, m_height;
	};

	//struct Sampler
	//{
	//	static std::unique_ptr<Texture> Create(TextureType type, Texture* boundTexture = nullptr);
	//	void BindTexture(Texture* boundTexture);
	//protected:
	//	Sampler(TextureType type, Texture* boundTexture);

	//	Texture* m_boundTexture;
	//	TextureType m_type;
	//};
}