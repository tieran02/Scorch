#pragma once
#include "render/pipeline.h"

namespace SC
{
	enum class AttachmentLoadOp
	{
		LOAD,
		CLEAR,
		DONT_CARE
	};

	enum class AttachmentStoreOp
	{
		STORE,
		DONT_CARE,
	};

	enum class ImageLayout
	{
		UNDEFINED,
		COLOR_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		DEPTH_ATTACHMENT_OPTIMAL,
		DEPTH_READ_ONLY_OPTIMAL,
		SHADER_READ_ONLY_OPTIMAL,
		TRANSFER_SRC_OPTIMAL,
		TRANSFER_DST_OPTIMAL,
		PRESENT_SRC_KHR
	};

	struct RenderpassAttachment
	{
		Format format;
		AttachmentLoadOp loadOp;
		AttachmentStoreOp storeOp;
		AttachmentLoadOp stencilLoadOp;
		AttachmentStoreOp stencilStoreOp;
		ImageLayout initialLayout;
		ImageLayout finalLayout;
	};

	struct RenderpassAttachmentRef
	{
		uint32_t index;
		ImageLayout layout;
	};

	class Renderpass
	{
	public:
		virtual ~Renderpass();

		static std::unique_ptr<Renderpass> Create();
		virtual bool Build() = 0;

		void AddAttachment(RenderpassAttachment&& _layoutattachment);
		void AddColourReference(uint32_t attachmentIndex, ImageLayout format);
		void AddDepthReference(uint32_t attachmentIndex, ImageLayout format);
	protected:
		Renderpass();

		std::vector<RenderpassAttachment> m_attachments;
		std::vector<RenderpassAttachmentRef> m_colourReferences;
		std::optional<RenderpassAttachmentRef> m_depthReference;
	};
}