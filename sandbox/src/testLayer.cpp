#include "TestLayer.h"
#include "engine.h"


void TestLayer::OnAttach()
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/coloured_triangle.vert.spv")
		.SetFragmentModulePath("shaders/coloured_triangle.frag.spv")
		.Build();

	auto pipeline = SC::Pipeline::Create(*shader);
	pipeline->Build();
}

void TestLayer::OnDetach()
{

}

void TestLayer::OnUpdate()
{

}

void TestLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
