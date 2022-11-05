#include "MaterialLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "scorch/event/keyEvent.h"
#include "glm/gtx/norm.hpp"
#include "scorch/core/app.h"

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


MaterialLayer::MaterialLayer() : Layer("MaterialLayer"),
m_rotation(0),
m_pos(0.f, -0.5f, -4.f)
{

}

void MaterialLayer::OnAttach()
{
	m_rotation = 0;

	//create a pipeline layout with push constants
	m_shaderEffect = SC::ShaderEffect::Builder("shaders/textured.vert.spv", "shaders/lit.frag.spv")
		.AddPushConstant("modelPush", { {SC::ShaderStage::VERTEX}, sizeof(MeshPushConstants) })
		.AddSet("cameraData", { { SC::DescriptorBindingType::UNIFORM, {SC::ShaderStage::VERTEX} } })
		.AddSet("textureData",
			{
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Diffuse
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Spec
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Normal
				{ SC::DescriptorBindingType::SAMPLER, {SC::ShaderStage::FRAGMENT}}, //Mask
			})
		.Build();

	m_shaderPass.Build(m_shaderEffect);

	SC::EffectTemplate effectTemplate;
	effectTemplate.passShaders[SC::MeshpassType::Forward] = &m_shaderPass;
	m_materialSystem.AddEffectTemplate("default", effectTemplate);

	m_globalDescriptorSet = SC::FrameData<SC::DescriptorSet>::Create(m_shaderEffect.GetDescriptorSetLayout(0));


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

	CreateScene();
}

void MaterialLayer::OnDetach()
{

}

void MaterialLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	float moveSpeed = 300;
	glm::vec3 input{ 0,0,0 };
	if (SC::Input::IsKeyDown(KEY_LEFT_SHIFT))
	{
		moveSpeed *= 4.0f;
	}
	if (SC::Input::IsKeyDown(KEY_W))
	{
		input += m_camera.Front();
	}
	if (SC::Input::IsKeyDown(KEY_S))
	{
		input -= m_camera.Front();
	}
	if (SC::Input::IsKeyDown(KEY_A))
	{
		input -= m_camera.Right();
	}
	if (SC::Input::IsKeyDown(KEY_D))
	{
		input += m_camera.Right();
	}
	if (SC::Input::IsKeyDown(KEY_E))
	{
		input.y += 1.0f;
	}
	if (SC::Input::IsKeyDown(KEY_Q))
	{
		input.y -= 1.0f;
	}
	if (glm::length2(input) > 0)
		input = glm::normalize(input);
	m_camera.Translate(input * moveSpeed * deltaTime);
	float mouseX, mouseY;
	SC::Input::GetMousePos(mouseX, mouseY);
	m_camera.MousePosition(mouseX, mouseY);

	//write descriptors to camera buffer
	GPUCameraData camData;
	camData.proj = glm::perspective(glm::radians(m_camera.Fov()), (float)windowWidth / (float)windowHeight, 0.1f, 20000.0f);
	camData.proj[1][1] *= -1;
	camData.view = m_camera.GetViewMatrix();
	camData.viewproj = camData.proj * camData.view;

	for (const auto& buffer : m_cameraBuffer)
	{
		auto mappedData = buffer->Map();
		memcpy(mappedData.Data(), &camData, sizeof(GPUCameraData));
	}

	Draw();
}

void MaterialLayer::OnEvent(SC::Event& event)
{

}

void MaterialLayer::CreateScene()
{
	{
		std::vector<SC::Mesh> meshes;
		std::vector<std::string> names;
		std::vector<SC::MaterialInfo> materialData;
		SC::Mesh::LoadMeshesFromFile("models/sponza/sponza.obj", meshes, &names, &materialData, true);

		for (const auto& matData : materialData)
		{
			if (matData.textures.empty())
				continue;

			//Just use the first diffuse for now, TODO add other texture types e.g specular
			const std::string& diffusePath = matData.textures[0];
			if (diffusePath.empty() || m_textures.find(diffusePath) != m_textures.end())
				continue;

			auto texture = SC::Texture::Create(SC::TextureType::TEXTURE2D, SC::TextureUsage::COLOUR, SC::Format::R8G8B8A8_SRGB);
			texture->LoadFromFile(string_format("models/sponza/%s", diffusePath.c_str()));

			m_textures.emplace(diffusePath, std::move(texture));
		}

		bool created = false;
		for (int i = 0; i < meshes.size(); ++i)
		{
			SC::Mesh* mesh = m_scene.InsertMesh(names[i], std::move(meshes[i]));

			SC::RenderObject renderObject;
			renderObject.name = names[i];
			renderObject.mesh = mesh;

			auto matInfo = std::find_if(materialData.begin(), materialData.end(), [mesh](const SC::MaterialInfo& material)
				{
					return material.materialName == mesh->materialName;
				});
			CORE_ASSERT(matInfo, "Couldn't find material info");

			SC::MaterialData matData;
			if (matInfo != materialData.end())
			{
				matData.baseTemplate = "default";
				auto it = matInfo->textures.empty() ? m_textures.end() : m_textures.find(matInfo->textures[0]);
				SC::Texture* texture = it != m_textures.end() ? it->second.get() : SC::App::Instance()->GetRenderer()->WhiteTexture();
				matData.textures.push_back(texture);
			}

			SC::Material* material = m_materialSystem.BuildMaterial(mesh->materialName, matData);
			CORE_ASSERT(material, "Failed to build material");
			renderObject.material = material;
			m_scene.CreateRenderObject(std::move(renderObject));
		}
	}
}

void MaterialLayer::Draw()
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame(153.0f / 255.0f, 217.0f / 255.0f, 234.0f / 255.0f);

	//Not optimal as we create a viewport object each frame but will do for demo
	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

	m_scene.DrawObjects(renderer,
		[=](const SC::RenderObject& renderObject, bool pipelineChanged)
		{	//Per object func gets called on each render object
			MeshPushConstants constants;
			constants.render_matrix = renderObject.transform;

			auto shaderEffect = renderObject.material->original->passShaders[SC::MeshpassType::Forward]->GetShaderEffect();
			auto textureDescriptorSet = renderObject.material->passSets[SC::MeshpassType::Forward].get();

			renderer->PushConstants(shaderEffect->GetPipelineLayout(), 0, 0, sizeof(MeshPushConstants), &constants);
			renderer->BindDescriptorSet(shaderEffect->GetPipelineLayout(), textureDescriptorSet, 1);

			if (pipelineChanged) { //only bind camera descriptors if pipeline changed
				renderer->BindDescriptorSet(shaderEffect->GetPipelineLayout(), m_globalDescriptorSet.GetFrameData(renderer->FrameDataIndex()));
			}

		});

	renderer->EndFrame();
}
