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
	auto shader = shaderBuilder.SetVertexModulePath("data/shaders/tri_mesh.vert.spv")
		.SetFragmentModulePath("data/shaders/tri_mesh.frag.spv")
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
	m_pipeline->faceCulling = SC::FaceCulling::NONE;
	m_pipeline->Build();

	auto stride = m_pipeline->vertexInputDescription.GetStride();

	//test
	SC::BufferUsageSet bufferUsage;
	bufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	bufferUsage.set(SC::BufferUsage::MAP);

	//make the array 3 vertices long
	SC::Mesh triangleMesh;
	triangleMesh.vertices.resize(3);

	//vertex positions
	triangleMesh.vertices[0].position = { 1.f, 1.f, 0.0f };
	triangleMesh.vertices[1].position = { -1.f, 1.f, 0.0f };
	triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);
	m_vertexBuffer = SC::Buffer::Create(triangleMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE, triangleMesh.vertices.data());
}

void VertexBufferLayer::OnDetach()
{

}

void VertexBufferLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

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
	SC::CommandBuffer& commandBuffer = renderer->GetFrameCommandBuffer();

	renderer->BeginFrame();

	commandBuffer.ResetCommands();
	commandBuffer.BeginRecording();

	commandBuffer.BeginRenderPass(renderer->DefaultRenderPass(), renderer->DefaultRenderTarget(), 153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f);

	commandBuffer.SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	commandBuffer.SetScissor(SC::Scissor(windowWidth, windowHeight));

	commandBuffer.BindPipeline(m_pipeline.get());
	commandBuffer.BindVertexBuffer(m_vertexBuffer.get());

	commandBuffer.PushConstants(m_pipelineLayout.get(), 0, 0, sizeof(MeshPushConstants), &constants);

	commandBuffer.Draw(3, 1, 0, 0);

	commandBuffer.EndRenderPass();

	commandBuffer.EndRecording();

	renderer->SubmitCommandBuffer(commandBuffer);

	renderer->EndFrame();
}

void VertexBufferLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}
