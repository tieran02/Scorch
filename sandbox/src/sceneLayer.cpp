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

	float gTime;
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
m_lightDir(glm::vec4(0.44f, 0.89f, 0.22f, 0)),
m_globalShiniess(1.0f),
m_globalSpecStrength(0.5f),
m_zoom(10.0f)
{

}

void SceneLayer::OnAttach()
{
	const SC::App* app = SC::App::Instance();
	m_gui = SC::GUI::Create(app->GetRenderer(), app->GetWindowHandle());

	m_shaderEffect = SC::ShaderEffect::Builder("data/shaders/diffuse.vert.spv", "data/shaders/diffuse.frag.spv")
		.AddPushConstant("modelPush", { {SC::ShaderStage::VERTEX}, sizeof(MeshPushConstants) })
		.AddSet("textureData",
			{
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Diffuse
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Spec
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Alpha
				{ SC::DescriptorBindingType::UNIFORM, {SC::ShaderStage::FRAGMENT}}, //Shader Data
			})
			.SetTextureSetIndex(0)
		.AddSet("sceneData", { { SC::DescriptorBindingType::UNIFORM, {SC::ShaderStage::VERTEX, SC::ShaderStage::FRAGMENT} } })
		.Build();

	m_shaderPass.Build(m_shaderEffect);

	m_sceneDescriptorSet = SC::FrameData<SC::DescriptorSet>::Create(m_shaderEffect.GetDescriptorSetLayout(1));
	for (uint8_t i = 0; i < m_sceneDescriptorSet.FrameCount(); ++i)
	{
		SC::Buffer* buffer = m_scene.GetSceneUniformBuffer(i);
		SC::DescriptorSet* descriptorSet = m_sceneDescriptorSet.GetFrameData(i);

		descriptorSet->SetBuffer(buffer, 0);
	}

	SC::EffectTemplate effectTemplate;
	effectTemplate.passShaders[SC::MeshpassType::Forward] = &m_shaderPass;
	m_materialSystem.AddEffectTemplate("default", effectTemplate);

	helmetRoot = m_scene.LoadModel("data/models/helmet/DamagedHelmet.modl", &m_materialSystem);
	helmetRoot->GetTransform().SetRotation(glm::vec3(1, 0, 0), glm::radians(90.0f));
	helmetRoot->GetTransform().SetScale(glm::vec3(3.0f));

	sponzaRoot = m_scene.LoadModel("data/models/sponza/sponza.modl", &m_materialSystem);

	m_scene.GetSceneData().Lights[0].intensities = glm::vec4(0.9f, 0.6f, 0.4f, 1.0f);
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

	glm::vec3 camPos = { sinf(gTime) * m_zoom, -0.05f, cosf(gTime) * m_zoom };
	m_scene.GetSceneData().EyePos = glm::vec4(camPos,1.0f);

	glm::mat4 view = glm::lookAt(camPos, glm::vec3(0), glm::vec3(0, 1, 0)); //glm::translate(glm::mat4(1.f), camPos);
	//camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.f), (float)windowWidth / (float)windowHeight, 0.1f, 1500.0f);
	projection[1][1] *= -1;
	m_scene.GetSceneData().ViewMatrix = projection * view;

	m_scene.GetSceneData().Lights[0].position = glm::normalize(m_lightDir);
	gTime += deltaTime * 0.5f;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame();

	renderer->BeginRenderPass(nullptr, nullptr, .4f, .4f, .4f);

	//Not optimal as we create a viewport object each frame but will do for demo
	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

	renderer->BindPipeline(m_shaderPass.GetPipeline());

	m_scene.Root().UpdateSelfAndChildren();

	m_scene.DrawObjects(renderer, [=](const SC::RenderObject& renderObject, bool pipelineChanged)
		{	//Per object func gets called on each render object

			auto shaderEffect = renderObject.material->original->passShaders[SC::MeshpassType::Forward]->GetShaderEffect();
			auto textureDescriptorSet = renderObject.material->passSets[SC::MeshpassType::Forward].GetFrameData(renderer->FrameDataIndex());

			//Update matieral paramter buffers
			renderObject.material->parameters.Update(renderer->FrameDataIndex());

			renderer->BindDescriptorSet(shaderEffect->GetPipelineLayout(), textureDescriptorSet, 0);

			MeshPushConstants constants;
			constants.render_matrix = (*renderObject.transform);
			renderer->PushConstants(m_shaderEffect.GetPipelineLayout(), 0, 0, sizeof(MeshPushConstants), &constants);

			if (pipelineChanged) { //only bind camera descriptors if pipeline changed
				renderer->BindDescriptorSet(shaderEffect->GetPipelineLayout(), m_sceneDescriptorSet.GetFrameData(renderer->FrameDataIndex()), 1);
			}
		});


	m_gui->BeginFrame();

	ImGui::Begin("Material");
	for (const auto& mat : m_materialSystem.Materials())
	{
		if (ImGui::CollapsingHeader(mat.first.c_str(), ImGuiTreeNodeFlags_None))
		{
			for (auto& paramters : mat.second->parameters.GetRegister())
			{
				switch (paramters.second.first)
				{
				case SC::ShaderParamterTypes::FLOAT:
					ImGui::InputFloat(paramters.first.c_str(), (float*)paramters.second.second);
					break;
				case SC::ShaderParamterTypes::INT:
					ImGui::InputInt(paramters.first.c_str(), (int*)paramters.second.second);
					break;
				case SC::ShaderParamterTypes::VEC3:
					ImGui::InputFloat3(paramters.first.c_str(), (float*)paramters.second.second);
					break;
				case SC::ShaderParamterTypes::VEC4:
					ImGui::InputFloat4(paramters.first.c_str(), (float*)paramters.second.second);
					break;
				}
			}	
		}
	}
	ImGui::End();
	ImGui::Begin("Global Material");
	ImGui::SliderFloat4("Light Dir", (float*)&m_lightDir.x, -1, 1);
	ImGui::End();

	ImGui::Begin("Camera");
	ImGui::SliderFloat("Zoom", &m_zoom, 1.0f, 100.0f);
	ImGui::End();

	m_gui->EndFrame();

	renderer->EndRenderPass();
	renderer->EndFrame();
}