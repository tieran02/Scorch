#include "pch.h"
#include "render/renderpass.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanRenderpass.h"

using namespace SC;

Renderpass::Renderpass() : m_depthReference{ std::nullopt }
{

}

Renderpass::~Renderpass()
{

}

std::unique_ptr<Renderpass> Renderpass::Create()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<Renderpass> renderPass{ nullptr };
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		renderPass = std::unique_ptr<Renderpass>(new VulkanRenderpass());
	}

	CORE_ASSERT(renderPass, "failed to create renderPass");
	return std::move(renderPass);
}

void Renderpass::AddAttachment(RenderpassAttachment&& attachment)
{
	m_attachments.emplace_back(std::move(attachment));
}

void Renderpass::AddColourReference(uint32_t attachmentIndex, ImageLayout format)
{
	m_colourReferences.emplace_back(attachmentIndex, format);
}

void Renderpass::AddDepthReference(uint32_t attachmentIndex, ImageLayout format)
{
	CORE_ASSERT(!m_depthReference.has_value(), "depth ref already exists");
	m_depthReference.emplace(attachmentIndex, format);
}
