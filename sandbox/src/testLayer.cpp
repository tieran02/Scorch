#include "TestLayer.h"

void TestLayer::OnAttach()
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("data/shaders/coloured_triangle.vert.spv")
		.SetFragmentModulePath("data/shaders/coloured_triangle.frag.spv")
		.Build();


	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->Build();

}

void TestLayer::OnDetach()
{

}

void TestLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	SC::CommandBuffer& commandBuffer = renderer->GetFrameCommandBuffer();

	renderer->BeginFrame();

	commandBuffer.ResetCommands();
	commandBuffer.BeginRecording();

	commandBuffer.BeginRenderPass(renderer->DefaultRenderPass(), renderer->DefaultRenderTarget(), 153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f);

	//Not optimal as we create a viewport object each frame but will do for demo
	commandBuffer.SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	commandBuffer.SetScissor(SC::Scissor(windowWidth, windowHeight));

	commandBuffer.BindPipeline(m_pipeline.get());
	commandBuffer.Draw(3, 1, 0, 0);

	commandBuffer.EndRenderPass();

	commandBuffer.EndRecording();

	renderer->SubmitCommandBuffer(commandBuffer);

	renderer->EndFrame();
}

void TestLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
