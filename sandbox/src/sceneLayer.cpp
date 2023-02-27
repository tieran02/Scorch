#include "sceneLayer.h"
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


SceneLayer::SceneLayer() : Layer("SceneLayer"),
m_rotation(0)
{

}

void SceneLayer::OnAttach()
{
	m_rotation = 0;

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

	m_scene.LoadModel("data/models/sponza/sponza.modl", &m_materialSystem);
}

void SceneLayer::OnDetach()
{
	m_scene.Reset();
}

void SceneLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

	glm::vec3 camPos = { 0.f,-0.05f,-2.2f };

	glm::mat4 view = glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)windowWidth / (float)windowHeight, 0.1f, 1500.0f);
	projection[1][1] *= -1;
	//model rotation
	glm::mat4 model = glm::rotate(glm::radians(m_rotation), glm::vec3(0, 1, 0));
	m_rotation += deltaTime * 20.0f;

	//calculate final mesh matrix
	glm::mat4 mesh_matrix = projection * view * model;

	MeshPushConstants constants;
	constants.render_matrix = mesh_matrix;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame(.4f, .4f, .4f);

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

	renderer->BindPipeline(m_shaderPass.GetPipeline());
	renderer->PushConstants(m_shaderEffect.GetPipelineLayout(), 0, 0, sizeof(MeshPushConstants), &constants);

	m_scene.DrawObjects(renderer, [=](const SC::RenderObject& renderObject, bool pipelineChanged)
		{	//Per object func gets called on each render object

			auto shaderEffect = renderObject.material->original->passShaders[SC::MeshpassType::Forward]->GetShaderEffect();
			auto textureDescriptorSet = renderObject.material->passSets[SC::MeshpassType::Forward].get();

			renderer->BindDescriptorSet(shaderEffect->GetPipelineLayout(), textureDescriptorSet, 0);
		});

	renderer->EndFrame();
}