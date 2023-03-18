#pragma once
#include "scorch/render/pipeline.h"

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

		virtual bool Build(uint32_t width, uint32_t height, bool generateMipmaps) = 0;
		virtual bool LoadFromFile(const std::string& path) = 0;
		virtual bool CopyData(const void* data, size_t size) = 0;

		Format GetFormat() const;
	protected:
		bool ReadImageFromFile(const std::string& path, ImageData& imageData);

		Texture(TextureType type, TextureUsage usage, Format format);
		TextureType m_type;
		TextureUsage m_usage;
		Format m_format;
		uint32_t m_width, m_height;
	};

	class Renderpass;
	struct RenderTarget
	{
	public:
		static std::unique_ptr<RenderTarget> Create(std::vector<Format>&& attachmentFormats, uint32_t width, uint32_t height);
		virtual ~RenderTarget();

		bool BuildAttachmentTexture(uint32_t attachmentIndex);
		bool SetAttachmentTexture(uint32_t attachmentIndex, Texture* texture);

		const Texture* GetAttachmentTexture(uint32_t attachmentIndex);

		virtual bool Build(Renderpass* renderPass) = 0;

		inline uint32_t GetWidth() const { return m_width; }
		inline uint32_t GetHeight() const { return m_height; }

	protected:
		RenderTarget(std::vector<Format>&& attachmentFormats, uint32_t width, uint32_t height);
	protected:
		std::vector<std::pair<Texture*, bool>> m_textures; //bool determines if the render target has ownership of the texture
														   //if the ownership is true the texture will be deleted in the RenderTarget destructor
		std::vector<Format> m_attachmentFormats;
		uint32_t m_width, m_height;
	};
}