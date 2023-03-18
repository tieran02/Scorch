#include "deferredLayer.h"

void DeferredLayer::OnAttach()
{
	SC::Log::Print("Creating Deferred Render pass");
	m_deferredRenderpass = SC::Renderpass::Create();

	SC::RenderpassAttachment colourAttachment;
	colourAttachment.format = SC::Format::B8G8R8A8_SRGB;
	colourAttachment.loadOp = SC::AttachmentLoadOp::CLEAR; // we Clear when this attachment is loaded
	colourAttachment.storeOp = SC::AttachmentStoreOp::STORE;// we keep the attachment stored when the renderpass ends
	colourAttachment.stencilLoadOp = SC::AttachmentLoadOp::DONT_CARE; 	//we don't care about stencil
	colourAttachment.stencilStoreOp = SC::AttachmentStoreOp::DONT_CARE;
	colourAttachment.initialLayout = SC::ImageLayout::UNDEFINED;
	colourAttachment.finalLayout = SC::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	m_deferredRenderpass->AddAttachment(std::move(colourAttachment));
	m_deferredRenderpass->AddColourReference(0, SC::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);


	SC::RenderpassAttachment normalAttachment;
	normalAttachment.format = SC::Format::B8G8R8A8_SRGB;
	normalAttachment.loadOp = SC::AttachmentLoadOp::CLEAR; // we Clear when this attachment is loaded
	normalAttachment.storeOp = SC::AttachmentStoreOp::STORE;// we keep the attachment stored when the renderpass ends
	normalAttachment.stencilLoadOp = SC::AttachmentLoadOp::DONT_CARE; 	//we don't care about stencil
	normalAttachment.stencilStoreOp = SC::AttachmentStoreOp::DONT_CARE;
	normalAttachment.initialLayout = SC::ImageLayout::UNDEFINED;
	normalAttachment.finalLayout = SC::ImageLayout::COLOR_ATTACHMENT_OPTIMAL;

	m_deferredRenderpass->AddAttachment(std::move(normalAttachment));
	m_deferredRenderpass->AddColourReference(1, SC::ImageLayout::COLOR_ATTACHMENT_OPTIMAL);


	bool success = m_deferredRenderpass->Build();
	SC::Log::Print(string_format("Deferred Render pass build result: {}", success), success ? SC::LogSeverity::LogInfo : SC::LogSeverity::LogFatel);


	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	SC::Log::Print("Creating render target");
	m_colourTarget = SC::RenderTarget::Create(std::vector{SC::Format::B8G8R8A8_SRGB, SC::Format::B8G8R8A8_SRGB}, windowWidth, windowHeight);
	m_colourTarget->BuildAttachmentTexture(0);
	m_colourTarget->BuildAttachmentTexture(1);
	success = m_colourTarget->Build(m_deferredRenderpass.get());
	SC::Log::Print(string_format("Render target build result: {}", success), success ? SC::LogSeverity::LogInfo : SC::LogSeverity::LogFatel);


	//Create pipeline layout
	m_pipelineLayout = SC::PipelineLayout::Create();
	SC::ColorComponentFlags componentFlags
	{
		SC::ColorComponentBits::COLOR_COMPONENT_R,
		SC::ColorComponentBits::COLOR_COMPONENT_G,
		SC::ColorComponentBits::COLOR_COMPONENT_B,
		SC::ColorComponentBits::COLOR_COMPONENT_A
	};

	m_pipelineLayout->AddColourBlendAttachment(componentFlags, false); //colour
	m_pipelineLayout->AddColourBlendAttachment(componentFlags, false); //normals
	m_pipelineLayout->Build();

	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("data/shaders/coloured_triangle.vert.spv")
		.SetFragmentModulePath("data/shaders/coloured_triangle_colour_normal.frag.spv")
		.Build();

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->pipelineLayout = m_pipelineLayout.get();
	m_pipeline->Build(m_deferredRenderpass.get());

}

void DeferredLayer::OnDetach()
{
	m_colourTarget.reset();
	m_deferredRenderpass.reset();
}

void DeferredLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame(.4f, .4f, .4f);

	//Hack for not, need to end default render pass
	renderer->EndRenderPass();

	//begin deferred render pass
	renderer->BeginRenderPass(m_deferredRenderpass.get(), m_colourTarget.get());

	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

	renderer->BindPipeline(m_pipeline.get());
	renderer->Draw(3, 1, 0, 0);

	renderer->EndFrame();
}
