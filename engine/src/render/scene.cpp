#include "pch.h"
#include "render/scene.h"
#include "render/renderer.h"
#include "render/pipeline.h"
#include "render/mesh.h"
#include "render/buffer.h"
#include "render/texture.h"
#include "assetModel.h"
#include "jaam.h"

using namespace SC;

namespace
{
	struct ModelUserData
	{
		std::vector<std::shared_ptr<Mesh>> meshes;
	};

	struct MaterialUserData
	{
		std::shared_ptr<Material> material;
	};

	struct TextureUserData
	{
		std::unique_ptr<Texture> texture;
	};

	Asset::TextureManager<TextureUserData> gTextureManager;
	Asset::ModelManager<ModelUserData> gModelManager;
	Asset::MaterialManager<MaterialUserData> gMaterialManager;

	struct BaseShaderUserData : ShaderUserData
	{
		float shininess{ 128 };
	};
}

Scene::Scene()
{
	m_sceneUbo.DirectionalLightDir = glm::vec4(0.44f, 0.89f,0.22f, 0);
	m_sceneUbo.DirectionalLightColor = glm::vec4(1, 1, 1, 1.2f);
	m_sceneUbo.ViewMatrix = glm::mat4(1.0f);
	m_sceneUbo.EyePos = glm::vec4(0.0f);

	CreateSceneUniformBuffers();
}

Scene::~Scene()
{
	Reset();
}

void Scene::DrawObjects(Renderer* renderer,
	std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc)
{
	CORE_ASSERT(renderer, "Renderer can't be null");

	//update current frames scene ubo
	UpdateSceneUniformBuffers(renderer->FrameDataIndex());

	PipelineLayout* lastLayout{ nullptr };
	m_root.TraverseTree([=, &lastLayout](SceneNode& node)
	{
		RenderObject& renderable = node.GetRenderObject();

		if (!renderable.mesh) return;

		auto* material = renderable.material;
		bool pipelineChanged = !lastLayout;
		if (material)
		{
			auto forwardEffect = material->original->passShaders[MeshpassType::Forward];
			pipelineChanged |= lastLayout != forwardEffect->GetShaderEffect()->GetPipelineLayout();

			if (pipelineChanged)
				renderer->BindPipeline(forwardEffect->GetPipeline());

			lastLayout = forwardEffect->GetShaderEffect()->GetPipelineLayout();
		}

		CORE_ASSERT(renderable.mesh, "Mesh can't be null");
		CORE_ASSERT(renderable.mesh->vertexBuffer, "Mesh vertex buffer can't be null, is it built?");
		CORE_ASSERT(renderable.mesh->vertexBuffer, "Mesh index buffer can't be null, is it built?");

		renderer->BindVertexBuffer(renderable.mesh->vertexBuffer.get());
		renderer->BindIndexBuffer(renderable.mesh->indexBuffer.get());

		PerRenderObjectFunc(renderable, pipelineChanged);

		renderer->DrawIndexed(renderable.mesh->IndexCount(), 1, 0, 0, 0);
	});
}

void Scene::Reset()
{
	m_root.Remove();

	m_meshes.clear();

	m_loadedModels.clear();
	m_loadedMaterial.clear();
	m_loadedTextures.clear();
}

SceneNode& Scene::Root()
{
	return m_root;
}

SceneNode* Scene::LoadModel(const std::string& path, MaterialSystem* materialSystem)
{
	//SceneNode sceneNode;
	std::shared_ptr<SceneNode> modelRoot = m_root.AddChild();

	gTextureManager.SetOnLoadCallback([=](const Asset::TextureInfo& textureInfo, auto& userData)
		{
			userData.texture = SC::Texture::Create(SC::TextureType::TEXTURE2D, SC::TextureUsage::COLOUR, SC::Format::R8G8B8A8_SRGB);
			userData.texture->Build(textureInfo.pixelsize[0], textureInfo.pixelsize[1], true);
			userData.texture->CopyData(textureInfo.data.data(), textureInfo.data.size());
		});

	gTextureManager.SetOnUnloadCallback([=](auto& userData)
		{
			userData.texture.reset();
		});

	gMaterialManager.SetOnLoadCallback([=](const Asset::MaterialInfo& matInfo, auto& userData)
		{
			SC::MaterialData matData;
			matData.baseTemplate = "default";

			std::vector<std::pair<std::string, Texture*>> texturesToBind //texture is the fallback default texture
			{
				{"baseColor", App::Instance()->GetRenderer()->WhiteTexture()},
				{"spec",App::Instance()->GetRenderer()->WhiteTexture()},
				{"alpha",App::Instance()->GetRenderer()->BlackTexture()},
			};

			for (const auto& textureType : texturesToBind)
			{
				auto textureIt = matInfo.textures.find(textureType.first);
				if (textureIt != matInfo.textures.end())
				{
					Asset::AssetHandle textureHandle = gTextureManager.Load(textureIt->second, false);
					APP_ASSERT(textureHandle.IsValid(), "Failed to load file");
					auto textureInfo = gTextureManager.GetUserData(textureHandle);
					APP_ASSERT(textureInfo, "Failed to get texture");

					matData.textures.push_back(textureInfo->texture.get());

					if (m_loadedTextures.find(textureHandle) == m_loadedTextures.end())
						m_loadedTextures.insert(textureHandle);
				}
				else
				{
					//use default white texture
					matData.textures.push_back(textureType.second);
				}
			}

			//Add base shader data
			matData.parameters.userData = std::make_shared<BaseShaderUserData>();
			matData.parameters.userDataSize = sizeof(BaseShaderUserData);

			auto mat = materialSystem->BuildMaterial(matInfo.name, matData);
			userData.material = mat;
		});

	gMaterialManager.SetOnUnloadCallback([=](auto& userData)
		{
			userData.material = nullptr;
		});

	gModelManager.SetOnLoadCallback([=](const Asset::ModelInfo& modelInfo, auto& userData)
		{
		const size_t meshCount = modelInfo.meshes.size();
		userData.meshes.resize(meshCount);

		for (size_t i = 0; i < meshCount; i++)
		{
			Asset::Mesh assetMesh = modelInfo.meshes[i];

			//Check if mesh already exists in map using mesh name
			auto meshIt = m_meshes.find(modelInfo.meshNames[i]);

			std::shared_ptr<Mesh> mesh;
			if (meshIt != m_meshes.end())
			{
				mesh = meshIt->second;
				userData.meshes[i] = mesh;
			}
			else
			{
				//Create new mesh
				mesh = std::make_shared<Mesh>();
				m_meshes.emplace(modelInfo.meshNames[i], mesh);
				userData.meshes[i] = mesh;

				mesh->vertices.resize(assetMesh.vertexBuffer.GetVertexCount());
				memcpy(mesh->vertices.data(), assetMesh.vertexBuffer.data.data(), assetMesh.vertexBuffer.data.size());

				mesh->indices.resize(assetMesh.indexBuffer.size());
				memcpy(mesh->indices.data(), assetMesh.indexBuffer.data(), assetMesh.indexBuffer.size() * sizeof(SC::VertexIndexType));

				mesh->Build();

				mesh->vertices.resize(0);
			}

			std::shared_ptr<SceneNode> child = modelRoot->AddChild();
			child->GetRenderObject().mesh = mesh.get();
			child->GetRenderObject().transform = &child->ModelMatrix();

			//also load mat
			auto matHandle = gMaterialManager.Load(modelInfo.meshMaterials.at(i));
			CORE_ASSERT(matHandle.IsValid(), "Failed to load mat");
			auto mat = gMaterialManager.GetUserData(matHandle);
			CORE_ASSERT(mat, "Failed to get mat user data");
			CORE_ASSERT(mat->material, "Material is null");

			child->GetRenderObject().material = mat->material.get();
			m_loadedMaterial.insert(matHandle);
		}
		});

	gModelManager.SetOnUnloadCallback([=](auto& userData)
		{
			userData.meshes.clear();
		});

	Asset::AssetHandle modelHandle = gModelManager.Load(path.c_str(), false);
	CORE_ASSERT(modelHandle.IsValid(), "Failed to load file");
	auto modelInfo = gModelManager.GetUserData(modelHandle);
	CORE_ASSERT(modelInfo, "Failed to get model");


	if (m_loadedModels.find(modelHandle) == m_loadedModels.end())
		m_loadedModels.insert(modelHandle);


	std::map<uint64_t, std::vector<uint64_t>> nodeChildren;

	return modelRoot.get();
}

void Scene::CreateSceneUniformBuffers()
{
	SC::BufferUsageSet uboUsage;
	uboUsage.set(SC::BufferUsage::UNIFORM_BUFFER);
	uboUsage.set(SC::BufferUsage::MAP);
	m_sceneUniformBuffers = SC::FrameData<SC::Buffer>::Create(sizeof(SceneUbo), uboUsage, SC::AllocationUsage::HOST);
}

void Scene::UpdateSceneUniformBuffers(uint8_t frameDataIndex)
{
	auto mappedData = m_sceneUniformBuffers.GetFrameData(frameDataIndex)->Map();
	memcpy(mappedData.Data(), &m_sceneUbo, sizeof(SceneUbo));
}

Buffer* Scene::GetSceneUniformBuffer(uint8_t frameDataIndex)
{
	return m_sceneUniformBuffers.GetFrameData(frameDataIndex);
}

SceneUbo& Scene::GetSceneData()
{
	return m_sceneUbo;
}
