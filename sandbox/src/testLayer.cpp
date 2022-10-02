#include "TestLayer.h"

void TestLayer::OnAttach()
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/coloured_triangle.vert.spv")
		.SetFragmentModulePath("shaders/coloured_triangle.frag.spv")
		.Build();

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->Build();
}

void TestLayer::OnDetach()
{

}

void TestLayer::OnUpdate(float deltaTime)
{
	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame();
	renderer->BindPipeline(m_pipeline.get());
	renderer->Draw(3, 1, 0, 0);
	renderer->EndFrame();
}

void TestLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
