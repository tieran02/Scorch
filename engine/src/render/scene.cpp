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
}

Scene::~Scene()
{
	Reset();
}

void Scene::DrawObjects(Renderer* renderer,
	std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc)
{
	CORE_ASSERT(renderer, "Renderer can't be null");

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

Texture* Scene::CreateTexture(const std::string& path)
{
	auto foundIt = m_textures.find(path);
	if (foundIt != m_textures.end())
	{
		return foundIt->second.get();
	}

	std::unique_ptr<Texture> texture = Texture::Create(TextureType::TEXTURE2D, TextureUsage::COLOUR, Format::R8G8B8A8_SRGB);
	texture->LoadFromFile(path);
	auto it = m_textures.insert(std::make_pair(path, std::move(texture)));

	return it.first->second.get();
}

void Scene::Reset()
{
	m_root.Remove();

	m_meshes.clear();
	m_textures.clear();
	m_vertexBuffers.clear();
	m_indexBuffers.clear();

	m_loadedModels.clear();
	m_loadedMaterial.clear();
}

SceneNode& Scene::Root()
{
	return m_root;
}

namespace
{
	void ProcessNode(Scene& scene, const Asset::ModelInfo& model, std::map<uint64_t, std::vector<uint64_t>>& nodes, uint64_t index, std::shared_ptr<SceneNode>& parentNode)
	{
		std::shared_ptr<SceneNode> childNode = parentNode->AddChild();

		//const auto meshData = model.node_meshes.find(index);
		//const auto meshName = model.node_names.find(index);
		//if (meshData != model.node_meshes.end() && meshName != model.node_names.end())
		//{
		//	if (!meshData->second.mesh_path.empty()) 
		//	{
		//		Asset::AssetFile meshAsset;
		//		bool success = Asset::LoadBinaryFile(meshData->second.mesh_path.c_str(), meshAsset);
		//		CORE_ASSERT(success, "Failed to load file");
		//		Asset::MeshInfo meshInfo = Asset::ReadMeshInfo(&meshAsset);


		//		CORE_ASSERT(meshInfo.vertexSize == sizeof(SC::Vertex), "Vetex type size doesn't match");
		//		CORE_ASSERT(meshInfo.indexSize == sizeof(SC::VertexIndexType), "Index type size doesn't match");

		//		std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

		//		Mesh& mesh = scene.InsertMesh(meshName->second);
		//		mesh.vertices.resize(meshInfo.vertexBuferSize / meshInfo.vertexSize);
		//		mesh.indices.resize(meshInfo.indexBuferSize / meshInfo.indexSize);
		//		Asset::UnpackMesh(&meshInfo, meshAsset.binaryBlob.data(), meshAsset.binaryBlob.size(),
		//			reinterpret_cast<char*>(mesh.vertices.data()), reinterpret_cast<char*>(mesh.indices.data()));

		//		mesh.Build();

		//		childNode->GetRenderObject().mesh = &mesh;
		//	}
		//}



		////Process the children of this node
		//auto childrenIt = nodes.find(index);
		//if (childrenIt == nodes.end())
		//	return; //This node has no children 


		//const auto& children = nodes.at(index);
		//for (const auto& childIndex : children)
		//{
		//	ProcessNode(scene, model, nodes, childIndex, childNode);
		//}
	}
}

bool Scene::LoadModel(const std::string& path, MaterialSystem* materialSystem)
{
	//SceneNode sceneNode;
	std::shared_ptr<SceneNode> modelRoot = m_root.AddChild();

	gTextureManager.SetOnLoadCallback([=](const Asset::TextureInfo& textureInfo, auto& userData)
		{
			userData.texture = SC::Texture::Create(SC::TextureType::TEXTURE2D, SC::TextureUsage::COLOUR, SC::Format::R8G8B8A8_SRGB);
			userData.texture->Build(textureInfo.pixelsize[0], textureInfo.pixelsize[1]);
			userData.texture->CopyData(textureInfo.data.data(), textureInfo.data.size());
		});

	gMaterialManager.SetOnLoadCallback([=](const Asset::MaterialInfo& matInfo, auto& userData)
		{
			SC::MaterialData matData;
			matData.baseTemplate = "default";

			auto textureIt = matInfo.textures.find("baseColor");
			if (textureIt != matInfo.textures.end())
			{
				Asset::AssetHandle textureHandle = gTextureManager.Load(textureIt->second, false);
				APP_ASSERT(textureHandle.IsValid(), "Failed to load file");
				auto textureInfo = gTextureManager.GetUserData(textureHandle);
				APP_ASSERT(textureInfo, "Failed to get texture");

				matData.textures.push_back(textureInfo->texture.get());

				if(m_loadedTextures.find(textureHandle) == m_loadedTextures.end())
					m_loadedTextures.insert(textureHandle);
			}
			else
			{
				//use default white texture
				matData.textures.push_back(App::Instance()->GetRenderer()->WhiteTexture());

			}

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
			}

			std::shared_ptr<SceneNode> child = modelRoot->AddChild();
			child->GetRenderObject().mesh = mesh.get();

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

	//Get a map of parents and their children to create the scene graph
	/*for (const auto& currentNode : modelInfo.node_parents)
	{
		const auto nodeMeshIt = modelInfo.node_meshes.find(currentNode.first);
		if(nodeMeshIt == modelInfo.node_meshes.end())
			continue;

		if(nodeMeshIt->second.mesh_path.empty())
			continue;

		nodeChildren[currentNode.second].push_back(currentNode.first);
	}

	std::shared_ptr<SceneNode> modelRoot = m_root.AddChild();
	for (const auto& currentParent : nodeChildren)
	{
		ProcessNode(*this, modelInfo, nodeChildren, currentParent.first, modelRoot);
	}*/



	return false;
}
