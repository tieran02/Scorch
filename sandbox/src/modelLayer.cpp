#include "modelLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

struct MeshPushConstants
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};

ModelLayer::ModelLayer() : Layer("ModelLayer"),
m_vertexBuffer(nullptr),
m_indexBuffer(nullptr),
m_rotation(0)
{

}

void ModelLayer::OnAttach()
{
	m_rotation = 0;
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/normalMesh.vert.spv")
		.SetFragmentModulePath("shaders/normalMesh.frag.spv")
		.Build();


	//create a pipeline layout with push constants
	m_pipelineLayout = SC::PipelineLayout::Create();
	SC::ShaderModuleFlags pushConstantStages;
	pushConstantStages.set(to_underlying(SC::ShaderStage::VERTEX));
	m_pipelineLayout->AddPushConstant(pushConstantStages, sizeof(MeshPushConstants));
	m_pipelineLayout->Build();

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //pos
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //normal
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //color
	m_pipeline->pipelineLayout = m_pipelineLayout.get();
	m_pipeline->Build();

	auto stride = m_pipeline->vertexInputDescription.GetStride();

	std::vector<SC::RenderObject> model;
	constexpr bool USE_INDEX_BUFFER = true;
	bool success = SC::RenderObject::LoadFromFile("models/monkey_smooth.obj", model, USE_INDEX_BUFFER);
	m_monkeyMesh = model[0].mesh;

	//test
	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(to_underlying(SC::BufferUsage::VERTEX_BUFFER));
	vertexBufferUsage.set(to_underlying(SC::BufferUsage::MAP));

	m_vertexBuffer = SC::Buffer::Create(m_monkeyMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE);
	{
		auto mappedData = m_vertexBuffer->Map();
		memcpy(mappedData.Data(), m_monkeyMesh.vertices.data(), m_monkeyMesh.VertexSize());
	}

	if (USE_INDEX_BUFFER)
	{
		SC::BufferUsageSet indexBufferUsage;
		indexBufferUsage.set(to_underlying(SC::BufferUsage::INDEX_BUFFER));
		indexBufferUsage.set(to_underlying(SC::BufferUsage::MAP));

		m_indexBuffer = SC::Buffer::Create(m_monkeyMesh.IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE);
		{
			auto mappedData = m_indexBuffer->Map();
			memcpy(mappedData.Data(), m_monkeyMesh.indices.data(), m_monkeyMesh.IndexSize());
		}
	}
}

void ModelLayer::OnDetach()
{

}

void ModelLayer::OnUpdate(float deltaTime)
{
	glm::vec3 camPos = { 0.f,-0.05f,-2.2f };

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


	if(!m_indexBuffer)
		renderer->Draw(m_monkeyMesh.VertexCount(), 1, 0, 0);
	else
	{
		//using a index buffer
		renderer->BindIndexBuffer(m_indexBuffer.get());
		renderer->DrawIndexed(m_monkeyMesh.IndexCount(), 1, 0, 0, 0);
	}
	renderer->EndFrame();
}

void ModelLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
