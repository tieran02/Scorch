#include "vertexBufferLayer.h"

void VertexBufferLayer::OnAttach()
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/tri_mesh.vert.spv")
		.SetFragmentModulePath("shaders/tri_mesh.frag.spv")
		.Build();

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //pos
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //normal
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //color
	m_pipeline->Build();

	auto stride = m_pipeline->vertexInputDescription.GetStride();

	//test
	SC::BufferUsageSet bufferUsage;
	bufferUsage.set(to_underlying(SC::BufferUsage::VERTEX_BUFFER));
	bufferUsage.set(to_underlying(SC::BufferUsage::MAP));

	m_vertexBuffer = SC::Buffer::Create(sizeof(int) * 4, bufferUsage, SC::AllocationUsage::DEVICE);
	{
		//make the array 3 vertices long
		Mesh triangleMesh;
		triangleMesh._vertices.resize(3);

		//vertex positions
		triangleMesh._vertices[0].position = { 1.f, 1.f, 0.0f };
		triangleMesh._vertices[1].position = { -1.f, 1.f, 0.0f };
		triangleMesh._vertices[2].position = { 0.f,-1.f, 0.0f };

		//vertex colors, all green
		triangleMesh._vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
		triangleMesh._vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
		triangleMesh._vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

		auto mappedData = m_vertexBuffer->Map();
		memcpy(mappedData.Data(), triangleMesh._vertices.data(), triangleMesh._vertices.size() * sizeof(Vertex));
	}

	int a = 0;
}

void VertexBufferLayer::OnDetach()
{

}

void VertexBufferLayer::OnUpdate()
{
	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame();
	renderer->BindPipeline(m_pipeline.get());
	renderer->BindVertexBuffer(m_vertexBuffer.get());

	renderer->Draw(3, 1, 0, 0);
	renderer->EndFrame();
}

void VertexBufferLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
