#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include "descriptorSet.h"
#include "materialSystem.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	using VertexIndexType = uint32_t;
	struct Material;
	struct MaterialInfo;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
		glm::vec2 uv;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<VertexIndexType> indices;
		std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

		uint32_t VertexCount() const;
		uint32_t VertexSize() const;

		uint32_t IndexCount() const;
		uint32_t IndexSize() const;

		bool Build();

		static bool LoadMeshesFromFile(const std::string& path,
			std::vector<Mesh>& meshes,
			std::vector<std::string>* names,
			std::vector<MaterialInfo>* materialData,
			bool useIndexBuffer);
	};

	struct RenderObject
	{
		RenderObject();

		std::string name;
		Mesh* mesh;
		Material* material;
		glm::mat4 transform;
	};

	std::vector<RenderObject> LoadRenderObjectsFromModel(const std::string& path, std::vector<Mesh>& outMeshes, MaterialSystem& materialSystem, std::unordered_map<std::string, std::unique_ptr<SC::Texture>>& textures);
	std::vector<RenderObject> LoadRenderObjectsFromModel(const std::string& path, MaterialSystem& materialSystem, std::unordered_map<std::string, std::unique_ptr<SC::Texture>>& textures,
		std::function<Mesh&(const std::string& name)> func);

}