#include "sceneLayer.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "scorch/event/keyEvent.h"
#include "glm/gtx/norm.hpp"

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
m_rotation(0),
m_pos( 0.f,-0.5f,-4.f )
{

}

void SceneLayer::OnAttach()
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

	CreateScene();
}

void SceneLayer::OnDetach()
{

}

void SceneLayer::OnUpdate(float deltaTime)
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	float moveSpeed = 300;
	glm::vec3 input{0,0,0};
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
	if(glm::length2(input) > 0)
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

void SceneLayer::OnEvent(SC::Event& event)
{
	SC::Log::Print(event.ToString());
}

void SceneLayer::CreateScene()
{
	{
		std::vector<SC::Mesh> meshes;
		std::vector<std::string> names;
		SC::Mesh::LoadMeshesFromFile("models/sponza/sponza.obj", meshes, &names, true);

		for (int i = 0; i < meshes.size(); ++i)
		{
			SC::Mesh* mesh = m_scene.InsertMesh(names[i], std::move(meshes[i]));

			SC::RenderObject renderObject;
			renderObject.name = names[i];
			renderObject.mesh = mesh;

			SC::Material* material = m_scene.CreateMaterial(m_pipeline.get(), m_pipelineLayout.get(), "default");
			renderObject.material = material;

			m_scene.CreateRenderObject(std::move(renderObject));
		}
	}
}

void SceneLayer::Draw()
{
	const SC::App* app = SC::App::Instance();
	int windowWidth{ 0 }, windowHeight{ 0 };
	app->GetWindowExtent(windowWidth, windowHeight);

	if (windowWidth <= 0 && windowHeight <= 0)
		return;

	SC::Renderer* renderer = SC::App::Instance()->GetRenderer();
	renderer->BeginFrame(153.0f/255.0f, 217.0f / 255.0f, 234.0f / 255.0f);

	//Not optimal as we create a viewport object each frame but will do for demo
	renderer->SetViewport(SC::Viewport(0, 0, static_cast<float>(windowWidth), static_cast<float>(windowHeight)));
	renderer->SetScissor(SC::Scissor(windowWidth, windowHeight));

	m_scene.DrawObjects(renderer, 
		[=](const SC::RenderObject& renderObject, bool pipelineChanged)
		{	//Per object func gets called on each render object
			MeshPushConstants constants;
			constants.render_matrix = renderObject.transform;

			renderer->PushConstants(renderObject.material->pipelineLayout, 0, 0, sizeof(MeshPushConstants), &constants);
			if(pipelineChanged) //only bind camera descriptors if pipeline changed
				renderer->BindDescriptorSet(renderObject.material->pipelineLayout, m_globalDescriptorSet.GetFrameData(renderer->FrameDataIndex()));

		});

	renderer->EndFrame();
}
