#include "TestLayer.h"

void TestLayer::OnAttach()
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/coloured_triangle.vert.spv")
		.SetFragmentModulePath("shaders/coloured_triangle.frag.spv")
		.Build();

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->Build();

	//test
	SC::BufferUsageSet bufferUsage;
	bufferUsage.set(to_underlying(SC::BufferUsage::VERTEX_BUFFER));
	bufferUsage.set(to_underlying(SC::BufferUsage::MAP));

	auto buffer = SC::Buffer::Create(sizeof(int) * 4, bufferUsage, SC::AllocationUsage::HOST);
	{
		auto mappedData = buffer->Map();
	}

	int a = 0;
}

void TestLayer::OnDetach()
{

}

void TestLayer::OnUpdate()
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
