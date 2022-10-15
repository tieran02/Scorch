#include "vertexBufferLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct MeshPushConstants
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};

VertexBufferLayer::VertexBufferLayer() : Layer("VertexBufferLayer"),
	m_rotation(0)
{

}

void VertexBufferLayer::OnAttach()
{
	m_rotation = 0;
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/tri_mesh.vert.spv")
		.SetFragmentModulePath("shaders/tri_mesh.frag.spv")
		.Build();


	//create a pipeline layout with push constants
	m_pipelineLayout = SC::PipelineLayout::Create();
	SC::ShaderModuleFlags pushConstantStages;
	pushConstantStages.set(SC::ShaderStage::VERTEX);
	m_pipelineLayout->AddPushConstant(pushConstantStages, sizeof(MeshPushConstants));
	m_pipelineLayout->Build();

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //pos
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //normal
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //color
	m_pipeline->pipelineLayout = m_pipelineLayout.get();
	m_pipeline->Build();

	auto stride = m_pipeline->vertexInputDescription.GetStride();

	//test
	SC::BufferUsageSet bufferUsage;
	bufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	bufferUsage.set(SC::BufferUsage::MAP);

	m_vertexBuffer = SC::Buffer::Create(sizeof(int) * 4, bufferUsage, SC::AllocationUsage::DEVICE);
	{
		//make the array 3 vertices long
		SC::Mesh triangleMesh;
		triangleMesh.vertices.resize(3);

		//vertex positions
		triangleMesh.vertices[0].position = { 1.f, 1.f, 0.0f };
		triangleMesh.vertices[1].position = { -1.f, 1.f, 0.0f };
		triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

		//vertex colors, all green
		triangleMesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
		triangleMesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
		triangleMesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

		auto mappedData = m_vertexBuffer->Map();
		memcpy(mappedData.Data(), triangleMesh.vertices.data(), triangleMesh.vertices.size() * sizeof(SC::Vertex));
	}

	int a = 0;
}

void VertexBufferLayer::OnDetach()
{

}

void VertexBufferLayer::OnUpdate(float deltaTime)
{
	glm::vec3 camPos = { 0.f,-0.25f,-2.f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;
	//model rotation
	glm::mat4 model = glm::rotate(glm::radians(m_rotation), glm::vec3(0, 1, 0));
	m_rotation += deltaTime * 20.0f;

	//calculate final mesh matrix
	glm::mat4 mesh_matrix = projection * view * model;

	MeshPushConstants constants;
	constants.render_matrix = mesh_matrix;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame();
	renderer->BindPipeline(m_pipeline.get());
	renderer->BindVertexBuffer(m_vertexBuffer.get());

	renderer->PushConstants(m_pipelineLayout.get(), 0, 0, sizeof(MeshPushConstants), &constants);

	renderer->Draw(3, 1, 0, 0);
	renderer->EndFrame();
}

void VertexBufferLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
