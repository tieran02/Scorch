#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include "descriptorSet.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	using VertexIndexType = uint16_t;
	struct Material;
	struct MaterialData;

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
		std::string materialName;

		uint32_t VertexCount() const;
		uint32_t VertexSize() const;

		uint32_t IndexCount() const;
		uint32_t IndexSize() const;

		static bool LoadMeshesFromFile(const std::string& path,
			std::vector<Mesh>& meshes,
			std::vector<std::string>* names,
			std::vector<MaterialData>* materialData,
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

}