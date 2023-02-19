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
	Asset::ModelManagerBasic gModelManager;
}

RenderObject* Scene::CreateRenderObject(RenderObject&& object)
{
	auto node = m_root.AddChild();
	node->GetRenderObject() = std::move(object);

	return &node->GetRenderObject();
}

Mesh& Scene::InsertMesh(const std::string& name)
{
	auto foundIt = m_meshes.find(name);
	if (foundIt != m_meshes.end())
	{
		return foundIt->second;
	}

	Mesh& insertedMesh = m_meshes.emplace(std::piecewise_construct,
		std::forward_as_tuple(name),
		std::forward_as_tuple()).first->second;

	return insertedMesh;
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

		auto forwardEffect = renderable.material->original->passShaders[MeshpassType::Forward];
		const bool pipelineChanged = !lastLayout || lastLayout != forwardEffect->GetShaderEffect()->GetPipelineLayout();
		if (pipelineChanged)
			renderer->BindPipeline(forwardEffect->GetPipeline());

		CORE_ASSERT(renderable.mesh, "Mesh can't be null");
		CORE_ASSERT(renderable.mesh->vertexBuffer, "Mesh vertex buffer can't be null, is it built?");
		CORE_ASSERT(renderable.mesh->vertexBuffer, "Mesh index buffer can't be null, is it built?");

		renderer->BindVertexBuffer(renderable.mesh->vertexBuffer.get());
		renderer->BindIndexBuffer(renderable.mesh->indexBuffer.get());

		PerRenderObjectFunc(renderable, pipelineChanged);

		renderer->DrawIndexed(renderable.mesh->IndexCount(), 1, 0, 0, 0);

		lastLayout = forwardEffect->GetShaderEffect()->GetPipelineLayout();
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

	Asset::AssetHandle modelHandle = gModelManager.Load(path.c_str());
	CORE_ASSERT(modelHandle.IsValid(), "Failed to load file");
	Asset::ModelInfo* modelInfo = gModelManager.Get(modelHandle);
	CORE_ASSERT(modelInfo, "Failed to get model");

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
