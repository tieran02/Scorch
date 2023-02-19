#include "pch.h"
#include "render/mesh.h"
#include "render/materialSystem.h"
#include "jaam.h"
#include "render/texture.h"
#include "render/renderer.h"
#include "render/buffer.h"

using namespace SC;

namespace
{
	Asset::TextureManagerBasic gTextureManager;
	Asset::ModelManagerBasic gModelManager;
}

RenderObject::RenderObject() : mesh(nullptr), material(nullptr)
{

}

namespace
{
	//void LoadMaterials(const Asset::ModelInfo::NodeMesh& modelInfo, std::unordered_map<std::string, Asset::MaterialInfo>& loadedMats,
	//	std::unordered_map<std::string, std::unique_ptr<SC::Texture>>& textures)
	//{
	//	const auto& matPath = modelInfo.material_path;
	//	if (matPath.empty())
	//		return;

	//	if (loadedMats.find(matPath) != loadedMats.end())
	//		return;


	//	Asset::AssetFile materialAsset;
	//	Asset::LoadBinaryFile(matPath.c_str(), materialAsset);
	//	Asset::MaterialInfo matInfo = Asset::ReadMaterialInfo(&materialAsset);

	//	loadedMats.emplace(matPath, matInfo);

	//	//Just use the first diffuse for now, TODO add other texture types e.g specular
	//	const std::string& diffusePath = matInfo.textures["baseColor"];
	//	if (diffusePath.empty() || textures.find(diffusePath) != textures.end())
	//		return;

	//	Asset::AssetFile textureAsset;
	//	Asset::LoadBinaryFile(diffusePath.c_str(), textureAsset);
	//	if (textureAsset.json.empty())
	//		return;

	//	Asset::TextureInfo textureInfo = Asset::ReadTextureInfo(&textureAsset);

	//	auto texture = SC::Texture::Create(SC::TextureType::TEXTURE2D, SC::TextureUsage::COLOUR, SC::Format::R8G8B8A8_SRGB);
	//	texture->Build(textureInfo.pixelsize[0], textureInfo.pixelsize[1]);

	//	std::vector<char> pixelData(textureInfo.textureSize);
	//	Asset::UnpackTexture(&textureInfo, textureAsset.binaryBlob.data(), static_cast<int>(textureAsset.binaryBlob.size()), pixelData.data());

	//	texture->CopyData(pixelData.data(), pixelData.size());
	//	textures.emplace(diffusePath, std::move(texture));
	//}
}

uint32_t Mesh::VertexCount() const
{
	return static_cast<uint32_t>(vertices.size());
}

Mesh::Mesh() : vertexBuffer(nullptr), indexBuffer(nullptr)
{

}

uint32_t SC::Mesh::VertexSize() const
{
	return static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
}

bool Mesh::Build()
{
	if(vertexBuffer)
		Log::PrintCore("Mesh::Build: Vertex buffer already created, this will overwrite the existing buffer", LogSeverity::LogWarning);
	if (indexBuffer)
		Log::PrintCore("Mesh::Build: Index buffer already created, this will overwrite the existing buffer", LogSeverity::LogWarning);

	SC::BufferUsageSet vertexBufferUsage;
	vertexBufferUsage.set(SC::BufferUsage::VERTEX_BUFFER);
	vertexBufferUsage.set(SC::BufferUsage::TRANSFER_DST); //Transfer this to gpu only memory

	vertexBuffer = SC::Buffer::Create(VertexSize(), vertexBufferUsage, SC::AllocationUsage::DEVICE, vertices.data());

	SC::BufferUsageSet indexBufferUsage;
	indexBufferUsage.set(SC::BufferUsage::INDEX_BUFFER);
	indexBufferUsage.set(SC::BufferUsage::TRANSFER_DST);

	indexBuffer = SC::Buffer::Create(IndexSize(), indexBufferUsage, SC::AllocationUsage::DEVICE, indices.data());

	return vertexBuffer && indexBuffer;
}

std::vector<RenderObject> SC::LoadRenderObjectsFromModel(const std::string& path,
	MaterialSystem* materialSystem,
	std::unordered_map<std::string, std::unique_ptr<SC::Texture>>& textures,
	std::unordered_map<std::string, SC::Mesh>& meshes)
{
	std::vector<RenderObject> renderObjects;

	Asset::AssetHandle modelHandle = gModelManager.Load(path.c_str());
	CORE_ASSERT(modelHandle.IsValid(), "Failed to load file");
	Asset::ModelInfo* modelInfo = gModelManager.Get(modelHandle);
	CORE_ASSERT(modelInfo, "Failed to get model");

	//renderObjects.reserve(modelInfo.node_meshes.size());

	//std::unordered_map<std::string, Asset::MaterialInfo> loadedMats;
	//for (const auto& node : modelInfo.node_meshes)
	//{
	//	if (meshes.find(node.second.mesh_path) != meshes.end())
	//		continue;

	//	Asset::AssetFile meshAsset;
	//	Asset::LoadBinaryFile(node.second.mesh_path.c_str(), meshAsset);
	//	Asset::MeshInfo meshInfo = Asset::ReadMeshInfo(&meshAsset);

	//	CORE_ASSERT(meshInfo.vertexSize == sizeof(SC::Vertex), "Vetex type size doesn't match");
	//	CORE_ASSERT(meshInfo.indexSize == sizeof(SC::VertexIndexType), "Index type size doesn't match");

	//	std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

	//	Mesh mesh;
	//	mesh.vertices.resize(meshInfo.vertexBuferSize / meshInfo.vertexSize);
	//	mesh.indices.resize(meshInfo.indexBuferSize / meshInfo.indexSize);
	//	Asset::UnpackMesh(&meshInfo, meshAsset.binaryBlob.data(), meshAsset.binaryBlob.size(),
	//		reinterpret_cast<char*>(mesh.vertices.data()), reinterpret_cast<char*>(mesh.indices.data()));

	//	mesh.Build();

	//	//Add to mesh map (name -> mesh)
	//	auto meshIt = meshes.emplace(node.second.mesh_path, std::move(mesh));

	//	SC::RenderObject renderObject;
	//	renderObject.name = node.second.mesh_path;
	//	renderObject.mesh = &meshIt.first->second;

	//	if (materialSystem)
	//	{

	//		LoadMaterials(node.second, loadedMats, textures);

	//		auto materialIt = loadedMats.find(node.second.material_path);
	//		CORE_ASSERT(materialIt != loadedMats.end(), "Couldn't find material info");

	//		SC::MaterialData matData;
	//		if (materialIt != loadedMats.end())
	//		{
	//			matData.baseTemplate = "default";
	//			auto baseColourTexture = materialIt->second.textures.find("baseColor");
	//			if (baseColourTexture == materialIt->second.textures.end())
	//				matData.textures.push_back(SC::App::Instance()->GetRenderer()->WhiteTexture());


	//			auto it = materialIt->second.textures.empty() ? textures.end() : textures.find(baseColourTexture->second);
	//			SC::Texture* texture = it != textures.end() ? it->second.get() : SC::App::Instance()->GetRenderer()->WhiteTexture();
	//			matData.textures.push_back(texture);
	//		}

	//		SC::Material* material = materialSystem->BuildMaterial(node.second.material_path, matData);
	//		CORE_ASSERT(material, "Failed to build material");
	//		renderObject.material = material;
	//	}

	//	renderObjects.push_back(renderObject);
	//}
	return renderObjects;
}

bool LoadRenderObjectFromModel(const std::string& path, std::unordered_map<std::string, Mesh>& meshes, std::unordered_map<std::string, std::unique_ptr<Texture>>* textures, MaterialSystem* materialSystem)
{
	auto modelHandle = gModelManager.Load(path.c_str());
	CORE_ASSERT(modelHandle.IsValid(), "Failed to load file");
	Asset::ModelInfo* modelInfo = gModelManager.Get(modelHandle);
	CORE_ASSERT(modelInfo, "Failed to get model");

	//renderObjects.reserve(modelInfo.node_meshes.size());

	std::unordered_map<std::string, Asset::MaterialInfo> loadedMats;

	//for (const auto& node : modelInfo.node_meshes)
	//{
	//	if (meshes.find(node.second.mesh_path) != meshes.end())
	//		continue;

	//	Asset::AssetFile meshAsset;
	//	Asset::LoadBinaryFile(node.second.mesh_path.c_str(), meshAsset);
	//	Asset::MeshInfo meshInfo = Asset::ReadMeshInfo(&meshAsset);

	//	CORE_ASSERT(meshInfo.vertexSize == sizeof(SC::Vertex), "Vetex type size doesn't match");
	//	CORE_ASSERT(meshInfo.indexSize == sizeof(SC::VertexIndexType), "Index type size doesn't match");

	//	std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

	//	Mesh mesh;
	//	mesh.vertices.resize(meshInfo.vertexBuferSize / meshInfo.vertexSize);
	//	mesh.indices.resize(meshInfo.indexBuferSize / meshInfo.indexSize);
	//	Asset::UnpackMesh(&meshInfo, meshAsset.binaryBlob.data(), meshAsset.binaryBlob.size(),
	//		reinterpret_cast<char*>(mesh.vertices.data()), reinterpret_cast<char*>(mesh.indices.data()));

	//	mesh.Build();

	//	//Add to mesh map (name -> mesh)
	//	auto meshIt = meshes.emplace(node.second.mesh_path, std::move(mesh));

	//	SC::RenderObject renderObject;
	//	renderObject.name = node.second.mesh_path;
	//	renderObject.mesh = &meshIt.first->second;

	//	if (materialSystem && textures)
	//	{
	//		LoadMaterials(node.second, loadedMats, *textures);

	//		auto materialIt = loadedMats.find(node.second.material_path);
	//		CORE_ASSERT(materialIt != loadedMats.end(), "Couldn't find material info");

	//		SC::MaterialData matData;
	//		if (materialIt != loadedMats.end())
	//		{
	//			matData.baseTemplate = "default";
	//			auto baseColourTexture = materialIt->second.textures.find("baseColor");
	//			if (baseColourTexture == materialIt->second.textures.end())
	//				matData.textures.push_back(SC::App::Instance()->GetRenderer()->WhiteTexture());


	//			auto it = materialIt->second.textures.empty() ? textures->end() : textures->find(baseColourTexture->second);
	//			SC::Texture* texture = it != textures->end() ? it->second.get() : SC::App::Instance()->GetRenderer()->WhiteTexture();
	//			matData.textures.push_back(texture);
	//		}

	//		SC::Material* material = materialSystem->BuildMaterial(node.second.material_path, matData);
	//		CORE_ASSERT(material, "Failed to build material");
	//		renderObject.material = material;
	//	}

	//	//renderObjects.push_back(std::move(renderObject));
	//}

	return true;
}

uint32_t Mesh::IndexCount() const
{
	return static_cast<uint32_t>(indices.size());
}

uint32_t Mesh::IndexSize() const
{
	return static_cast<uint32_t>(indices.size() * sizeof(VertexIndexType));
}
