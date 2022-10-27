#include "descriptorLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "scorch/event/keyEvent.h"

struct MeshPushConstants
{
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct GPUCameraData
{
	glm::mat4 view;
	glm::mat4 proj;
	glm::mat4 viewproj;
};


DescriptorLayer::DescriptorLayer() : Layer("DescriptorLayer"),
m_vertexBuffer(nullptr),
m_indexBuffer(nullptr),
m_rotation(0),
m_pos( 0.f,-0.5f,-4.f )
{

}

void DescriptorLayer::OnAttach()
{
	m_rotation = 0;
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/descriptorMesh.vert.spv")
		.SetFragmentModulePath("shaders/descriptorMesh.frag.spv")
		.Build();


	//create a pipeline layout with push constants
	m_pipelineLayout = SC::PipelineLayout::Create();
	SC::ShaderModuleFlags pushConstantStages;
	pushConstantStages.set(SC::ShaderStage::VERTEX);
	m_pipelineLayout->AddPushConstant(pushConstantStages, sizeof(MeshPushConstants));

	m_setLayout = SC::DescriptorSetLayout::Create({ { SC::DescriptorBindingType::UNIFORM, pushConstantStages } });
	m_pipelineLayout->AddDescriptorSetLayout(m_setLayout.get());
	m_pipelineLayout->Build();

	m_globalDescriptorSet = SC::FrameData<SC::DescriptorSet>::Create(m_setLayout.get());

	m_pipeline = SC::Pipeline::Create(*shader);
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //pos
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //normal
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //color
	m_pipeline->pipelineLayout = m_pipelineLayout.get();
	m_pipeline->Build();

	auto stride = m_pipeline->vertexInputDescription.GetStride();

	std::vector<SC::Mesh> model;
	constexpr bool USE_INDEX_BUFFER = true;
	bool success = SC::Mesh::LoadMeshesFromFile("models/monkey_smooth.obj", model, nullptr, nullptr, USE_INDEX_BUFFER);
	m_monkeyMesh = model[0];

	//test
	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::MAP);

	m_vertexBuffer = SC::Buffer::Create(m_monkeyMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE);
	{
		auto mappedData = m_vertexBuffer->Map();
		memcpy(mappedData.Data(), m_monkeyMesh.vertices.data(), m_monkeyMesh.VertexSize());
	}

	if (USE_INDEX_BUFFER)
	{
		SC::BufferUsageSet indexBufferUsage;
		indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
		indexBufferUsage.set(SC::BufferUsage::MAP);

		m_indexBuffer = SC::Buffer::Create(m_monkeyMesh.IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE);
		{
			auto mappedData = m_indexBuffer->Map();
			memcpy(mappedData.Data(), m_monkeyMesh.indices.data(), m_monkeyMesh.IndexSize());
		}
	}

	//Upload camera data to uniform buffer for each overlapping frame using FrameData
	SC::BufferUsageSet cameraBufferUsage;
	cameraBufferUsage.set(SC::BufferUsage::UNIFORM_BUFFER);
	cameraBufferUsage.set(SC::BufferUsage::MAP);
	m_cameraBuffer = SC::FrameData<SC::Buffer>::Create(sizeof(GPUCameraData), cameraBufferUsage, SC::AllocationUsage::DEVICE);
	for (const auto& val : m_cameraBuffer) 
	{
		auto mappedData = val->Map();
		GPUCameraData cameraData{};
		memcpy(mappedData.Data(), &cameraData, sizeof(GPUCameraData));
	}

	//write descriptors to camera buffer
	//camera view
	glm::vec3 camPos = { 0.f,-0.5f,-4.f };
	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
	projection[1][1] *= -1;

	GPUCameraData camData;
	camData.proj = projection;
	camData.view = view;
	camData.viewproj = projection * view;

	for (uint8_t i = 0; i < m_globalDescriptorSet.FrameCount(); ++i)
	{
		SC::Buffer* buffer = m_cameraBuffer.GetFrameData(i);
		SC::DescriptorSet* descriptorSet = m_globalDescriptorSet.GetFrameData(i);

		descriptorSet->SetBuffer(buffer, 0);
		
		auto mappedData = buffer->Map();
		memcpy(mappedData.Data(), &camData, sizeof(GPUCameraData));
	}
}

void DescriptorLayer::OnDetach()
{

}

void DescriptorLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	m_rotation += deltaTime * 20.0f;

	constexpr float moveSpeed = 5;
	if (SC::Input::IsKeyDown(KEY_W))
	{
		m_pos.y -= moveSpeed * deltaTime;
	}
	if (SC::Input::IsKeyDown(KEY_S))
	{
		m_pos.y += moveSpeed * deltaTime;
	}
	if (SC::Input::IsKeyDown(KEY_A))
	{
		m_pos.x += moveSpeed * deltaTime;
	}
	if (SC::Input::IsKeyDown(KEY_D))
	{
		m_pos.x -= moveSpeed * deltaTime;
	}

	//write descriptors to camera buffer
	GPUCameraData camData;
	camData.proj = glm::perspective(glm::radians(70.f), (float)windowWidth / (float)windowHeight, 0.1f, 200.0f);
	camData.proj[1][1] *= -1;
	camData.view = glm::translate(glm::mat4(1.f), m_pos);;
	camData.viewproj = camData.proj * camData.view;

	for (const auto& buffer : m_cameraBuffer)
	{
		auto mappedData = buffer->Map();
		memcpy(mappedData.Data(), &camData, sizeof(GPUCameraData));
	}

	Draw();
}

void DescriptorLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}

void DescriptorLayer::Draw()
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

	//calculate final mesh matrix
	MeshPushConstants constants;
	glm::mat4 model = glm::rotate(glm::radians(m_rotation), glm::vec3(0, 1, 0));
	constants.render_matrix = model;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame();

	renderer->BindPipeline(m_pipeline.get());

	//Not optimal as we create a viewport object each frame but will do for demo
	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

	renderer->BindVertexBuffer(m_vertexBuffer.get());
	renderer->PushConstants(m_pipelineLayout.get(), 0, 0, sizeof(MeshPushConstants), &constants);
	renderer->BindDescriptorSet(m_pipelineLayout.get(), m_globalDescriptorSet.GetFrameData(renderer->FrameDataIndex()));


	if (!m_indexBuffer)
		renderer->Draw(m_monkeyMesh.VertexCount(), 1, 0, 0);
	else
	{
		//using a index buffer
		renderer->BindIndexBuffer(m_indexBuffer.get());
		renderer->DrawIndexed(m_monkeyMesh.IndexCount(), 1, 0, 0, 0);
	}
	renderer->EndFrame();
}
