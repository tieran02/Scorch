#include "modelLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "scorch/core/log.h"
#include "jaam.h"

namespace
{
	Asset::MaterialManagerBasic gMaterialManager;
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
m_rotation(0)
{

}

void ModelLayer::OnAttach()
{
	Asset::AssetHandle modelHandle = gModelManager.Load("data/models/helmet/DamagedHelmet.modl");
	//Asset::AssetHandle modelHandle = gModelManager.Load("data/models/Suzanne.modl");
	APP_ASSERT(modelHandle.IsValid(), "Failed to load file");
	Asset::ModelInfo* modelInfo = gModelManager.Get(modelHandle);
	APP_ASSERT(modelInfo, "Failed to get model");

	auto& mesh = modelInfo->meshes.at(0);
	m_monkeyMesh.vertices.resize(mesh.vertexBuffer.GetVertexCount());
	memcpy(m_monkeyMesh.vertices.data(), mesh.vertexBuffer.data.data(), mesh.vertexBuffer.data.size());

	m_monkeyMesh.indices.resize(mesh.indexBuffer.size());
	memcpy(m_monkeyMesh.indices.data(), mesh.indexBuffer.data(), mesh.indexBuffer.size() * sizeof(SC::VertexIndexType));

	m_rotation = 0;

	//test
	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);
	m_vertexBuffer = SC::Buffer::Create(m_monkeyMesh.VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE, m_monkeyMesh.vertices.data());

	SC::BufferUsageSet indexBufferUsage;
	indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
	indexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);
	m_indexBuffer = SC::Buffer::Create(m_monkeyMesh.IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE, m_monkeyMesh.indices.data());

#ifdef ModelLayer_UseMaterialSystem
	//create a pipeline layout with push constants
	m_shaderEffect = SC::ShaderEffect::Builder("data/shaders/diffuse.vert.spv", "data/shaders/diffuse.frag.spv")
		.AddPushConstant("modelPush", { {SC::ShaderStage::VERTEX}, sizeof(MeshPushConstants) })
		.AddSet("textureData",
			{
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Diffuse
			})
			.SetTextureSetIndex(0)
		.Build();

	m_shaderPass.Build(m_shaderEffect);

	SC::EffectTemplate effectTemplate;
	effectTemplate.passShaders[SC::MeshpassType::Forward] = &m_shaderPass;
	m_materialSystem.AddEffectTemplate("default", effectTemplate);

	//Load 
	Asset::AssetHandle matieralHandle = gMaterialManager.Load(modelInfo->meshMaterials[0]);
	APP_ASSERT(matieralHandle.IsValid(), "Failed to load file");
	Asset::MaterialInfo* matInfo = gMaterialManager.Get(matieralHandle);
	APP_ASSERT(matInfo, "Failed to get material");

	Asset::AssetHandle textureHandle = gTextureManager.Load(matInfo->textures.at("baseColor"));
	APP_ASSERT(textureHandle.IsValid(), "Failed to load file");
	Asset::TextureInfo* textureInfo = gTextureManager.Get(textureHandle);
	APP_ASSERT(textureInfo, "Failed to get texture");

	m_texture = SC::Texture::Create(SC::TextureType::TEXTURE2D, SC::TextureUsage::COLOUR, SC::Format::R8G8B8A8_SRGB);
	m_texture->Build(textureInfo->pixelsize[0], textureInfo->pixelsize[1]);
	m_texture->CopyData(textureInfo->data.data(), textureInfo->data.size());


	SC::MaterialData matData;
	matData.baseTemplate = "default";
	matData.textures.push_back(m_texture.get());
	m_materialSystem.BuildMaterial("monkey", matData);
#else
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
	glm::mat4 projection = glm::perspective(glm::radians(70.f),(float)windowWidth / (float)windowHeight, 0.1f, 1500.0f);
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

	renderer->BeginRenderPass(nullptr, nullptr, .4f, .4f, .4f);

	//upload camera data for this frame
	{
		auto frameIndex = renderer->FrameDataIndex();
		auto mappedData = m_cameraBuffer.GetFrameData(frameIndex)->Map();
		GPUCameraData cameraData{};
		memcpy(mappedData.Data(), &cameraData, sizeof(GPUCameraData));
	}

	//Not optimal as we create a viewport object each frame but will do for demo
	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

#ifdef ModelLayer_UseMaterialSystem
	SC::Material* mat = m_materialSystem.GetMaterial("monkey");
	auto forwardEffect = mat->original->passShaders[SC::MeshpassType::Forward];
	auto shaderEffect = forwardEffect->GetShaderEffect();
	auto textureDescriptorSet = mat->passSets[SC::MeshpassType::Forward].get();


	renderer->BindPipeline(forwardEffect->GetPipeline());
	renderer->PushConstants(shaderEffect->GetPipelineLayout(), 0, 0, sizeof(MeshPushConstants), &constants);
	renderer->BindDescriptorSet(shaderEffect->GetPipelineLayout(), textureDescriptorSet, 0);
#else
	renderer->BindPipeline(m_pipeline.get());
	renderer->PushConstants(m_pipelineLayout.get(), 0, 0, sizeof(MeshPushConstants), &constants);
#endif

	renderer->BindVertexBuffer(m_vertexBuffer.get());
	renderer->BindIndexBuffer(m_indexBuffer.get());
	renderer->DrawIndexed(m_monkeyMesh.IndexCount(), 1, 0, 0, 0);

	renderer->EndRenderPass();

	renderer->EndFrame();
}

void ModelLayer::OnEvent(SC::Event& event)
{
	//SC::Log::Print(event.ToString());
}
