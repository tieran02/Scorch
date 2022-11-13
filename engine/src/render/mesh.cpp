#include "pch.h"
#include "render/mesh.h"
#include "render/materialSystem.h"
#include "jaam.h"
#include "render/texture.h"
#include "render/renderer.h"
#include "render/buffer.h"

using namespace SC;

RenderObject::RenderObject() : transform(glm::mat4(1))
{

}

namespace
{
	void LoadMaterials(const Asset::ModelInfo::NodeMesh& modelInfo, std::unordered_map<std::string, Asset::MaterialInfo>& loadedMats,
		std::unordered_map<std::string, std::unique_ptr<SC::Texture>>& textures)
	{
		const auto& matPath = modelInfo.material_path;
		if (matPath.empty())
			return;

		if (loadedMats.find(matPath) != loadedMats.end())
			return;


		Asset::AssetFile materialAsset;
		Asset::LoadBinaryFile(matPath.c_str(), materialAsset);
		Asset::MaterialInfo matInfo = Asset::ReadMaterialInfo(&materialAsset);

		loadedMats.emplace(matPath, matInfo);

		//Just use the first diffuse for now, TODO add other texture types e.g specular
		const std::string& diffusePath = matInfo.textures["baseColor"];
		if (diffusePath.empty() || textures.find(diffusePath) != textures.end())
			return;

		Asset::AssetFile textureAsset;
		Asset::LoadBinaryFile(diffusePath.c_str(), textureAsset);
		if (textureAsset.json.empty())
			return;

		Asset::TextureInfo textureInfo = Asset::ReadTextureInfo(&textureAsset);

		auto texture = SC::Texture::Create(SC::TextureType::TEXTURE2D, SC::TextureUsage::COLOUR, SC::Format::R8G8B8A8_SRGB);
		texture->Build(textureInfo.pixelsize[0], textureInfo.pixelsize[1]);

		std::vector<char> pixelData(textureInfo.textureSize);
		Asset::UnpackTexture(&textureInfo, textureAsset.binaryBlob.data(), static_cast<int>(textureAsset.binaryBlob.size()), pixelData.data());

		texture->CopyData(pixelData.data(), pixelData.size());
		textures.emplace(diffusePath, std::move(texture));
	}
}

uint32_t Mesh::VertexCount() const
{
	return static_cast<uint32_t>(vertices.size());
}

uint32_t SC::Mesh::VertexSize() const
{
	return static_cast<uint32_t>(vertices.size() * sizeof(Vertex));
}

bool Mesh::Build()
{
	CORE_ASSERT(!vertexBuffer, "Vertex buffer already created, this will overwrite the existing buffer");
	CORE_ASSERT(!indexBuffer, "Index buffer already created, this will overwrite the existing buffer");

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

std::vector<RenderObject> SC::LoadRenderObjectsFromModel(const std::string& path, MaterialSystem* materialSystem, std::unordered_map<std::string, std::unique_ptr<SC::Texture>>& textures,
	std::function<Mesh& (const std::string& name)> func)
{
	std::vector<RenderObject> renderObjects;

	Asset::AssetFile modelAsset;
	Asset::LoadBinaryFile(path.c_str(), modelAsset);
	Asset::ModelInfo modelInfo = Asset::ReadModelInfo(&modelAsset);

	renderObjects.reserve(modelInfo.node_meshes.size());

	std::unordered_map<std::string, Asset::MaterialInfo> loadedMats;
	std::unordered_map<std::string, Asset::MeshInfo> loadedMeshes;
	for (const auto& node : modelInfo.node_meshes)
	{
		if (loadedMeshes.find(node.second.mesh_path) != loadedMeshes.end())
			continue;

		Asset::AssetFile meshAsset;
		Asset::LoadBinaryFile(node.second.mesh_path.c_str(), meshAsset);
		Asset::MeshInfo meshInfo = Asset::ReadMeshInfo(&meshAsset);

		loadedMeshes.emplace(node.second.mesh_path, meshInfo);

		CORE_ASSERT(meshInfo.vertexSize == sizeof(SC::Vertex), "Vetex type size doesn't match");
		CORE_ASSERT(meshInfo.indexSize == sizeof(SC::VertexIndexType), "Index type size doesn't match");

		std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

		Mesh& mesh = func(node.second.mesh_path);
		mesh.vertices.resize(meshInfo.vertexBuferSize / meshInfo.vertexSize);
		mesh.indices.resize(meshInfo.indexBuferSize / meshInfo.indexSize);
		Asset::UnpackMesh(&meshInfo, meshAsset.binaryBlob.data(), meshAsset.binaryBlob.size(),
			reinterpret_cast<char*>(mesh.vertices.data()), reinterpret_cast<char*>(mesh.indices.data()));

		mesh.Build();

		SC::RenderObject renderObject;
		renderObject.name = node.second.mesh_path;
		renderObject.mesh = &mesh;

		if (materialSystem)
		{

			LoadMaterials(node.second, loadedMats, textures);

			auto materialIt = loadedMats.find(node.second.material_path);
			CORE_ASSERT(materialIt != loadedMats.end(), "Couldn't find material info");

			SC::MaterialData matData;
			if (materialIt != loadedMats.end())
			{
				matData.baseTemplate = "default";
				auto baseColourTexture = materialIt->second.textures.find("baseColor");
				if (baseColourTexture == materialIt->second.textures.end())
					matData.textures.push_back(SC::App::Instance()->GetRenderer()->WhiteTexture());


				auto it = materialIt->second.textures.empty() ? textures.end() : textures.find(baseColourTexture->second);
				SC::Texture* texture = it != textures.end() ? it->second.get() : SC::App::Instance()->GetRenderer()->WhiteTexture();
				matData.textures.push_back(texture);
			}

			SC::Material* material = materialSystem->BuildMaterial(node.second.material_path, matData);
			CORE_ASSERT(material, "Failed to build material");
			renderObject.material = material;
		}

		renderObjects.push_back(std::move(renderObject));
	}
	return renderObjects;
}

uint32_t Mesh::IndexCount() const
{
	return static_cast<uint32_t>(indices.size());
}

uint32_t Mesh::IndexSize() const
{
	return static_cast<uint32_t>(indices.size() * sizeof(VertexIndexType));
}
