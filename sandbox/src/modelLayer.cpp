#include "modelLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "scorch/core/log.h"
#include "jaam.h"

namespace
{
	Asset::TextureManagerBasic gTextureManager;
	Asset::ModelManagerBasic gModelManager;
}

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


ModelLayer::ModelLayer() : Layer("ModelLayer"),
#ifndef ModelLayer_UseScene
m_vertexBuffer(nullptr),
m_indexBuffer(nullptr),
#endif
m_rotation(0)
{

}

void ModelLayer::OnAttach()
{
#ifndef ModelLayer_UseScene
	Asset::AssetHandle modelHandle = gModelManager.Load("data/models/monkey_smooth.modl");
	APP_ASSERT(modelHandle.IsValid(), "Failed to load file");
	Asset::ModelInfo* modelInfo = gModelManager.Get(modelHandle);
	APP_ASSERT(modelInfo, "Failed to get model");

	auto& mesh = modelInfo->meshes.at(0);
	m_monkeyMesh.vertices.resize(mesh.vertexBuffer.GetVertexCount());
	memcpy(m_monkeyMesh.vertices.data(), mesh.vertexBuffer.data.data(), mesh.vertexBuffer.data.size());

	m_monkeyMesh.indices.resize(mesh.indexBuffer.size());
	memcpy(m_monkeyMesh.indices.data(), mesh.indexBuffer.data(), mesh.indexBuffer.size() * sizeof(SC::VertexIndexType));
#else
	m_scene.LoadModel("data/models/monkey_smooth.modl", nullptr);
#endif // !ModelLayer_UseScene	


	m_rotation = 0;
	SC::ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("data/shaders/normalMesh.vert.spv")
		.SetFragmentModulePath("data/shaders/normalMesh.frag.spv")
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
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32_SFLOAT);	//UV
	m_pipeline->pipelineLayout = m_pipelineLayout.get();
	m_pipeline->Build();

	auto stride = m_pipeline->vertexInputDescription.GetStride();


#ifndef ModelLayer_UseScene
	constexpr bool USE_INDEX_BUFFER = true;

	//test
	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);
	m_vertexBuffer = SC::Buffer::Create(m_monkeyMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE, m_monkeyMesh.vertices.data());

	if (USE_INDEX_BUFFER)
	{
		SC::BufferUsageSet indexBufferUsage;
		indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
		indexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);
		m_indexBuffer = SC::Buffer::Create(m_monkeyMesh.IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE, m_monkeyMesh.indices.data());
	}
#endif

	//Upload camera data to uniform buffer for each overlapping frame using FrameData
	SC::BufferUsageSet cameraBufferUsage;
	cameraBufferUsage.set(SC::BufferUsage::UNIFORM_BUFFER);
	cameraBufferUsage.set(SC::BufferUsage::MAP);
	m_cameraBuffer = SC::FrameData<SC::Buffer>::Create(sizeof(GPUCameraData), cameraBufferUsage, SC::AllocationUsage::HOST);
	for (const auto& val : m_cameraBuffer) 
	{
		auto mappedData = val->Map();
		GPUCameraData cameraData{};
		memcpy(mappedData.Data(), &cameraData, sizeof(GPUCameraData));
	}

}

void ModelLayer::OnDetach()
{
	m_scene.Reset();
}

void ModelLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

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
	renderer->BeginFrame(.4f,.4f,.4f);

	//upload camera data for this frame
	{
		auto frameIndex = renderer->FrameDataIndex();
		auto mappedData = m_cameraBuffer.GetFrameData(frameIndex)->Map();
		GPUCameraData cameraData{};
		memcpy(mappedData.Data(), &cameraData, sizeof(GPUCameraData));
	}

	renderer->BindPipeline(m_pipeline.get());

	//Not optimal as we create a viewport object each frame but will do for demo
	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

	renderer->PushConstants(m_pipelineLayout.get(), 0, 0, sizeof(MeshPushConstants), &constants);

#ifndef ModelLayer_UseScene
	renderer->BindVertexBuffer(m_vertexBuffer.get());
	
	if(!m_indexBuffer)
		renderer->Draw(m_monkeyMesh.VertexCount(), 1, 0, 0);
	else
	{
		//using a index buffer
		renderer->BindIndexBuffer(m_indexBuffer.get());
		renderer->DrawIndexed(m_monkeyMesh.IndexCount(), 1, 0, 0, 0);
	}
#else
	m_scene.DrawObjects(renderer, [=](const SC::RenderObject& renderObject, bool pipelineChanged)
		{	//Per object func gets called on each render object
			
		});
#endif

	renderer->EndFrame();
}

void ModelLayer::OnEvent(SC::Event& event)
{
	//SC::Log::Print(event.ToString());
}
