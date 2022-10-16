#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	using VertexIndexType = uint16_t;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<VertexIndexType> indices;

		uint32_t VertexCount() const;
		uint32_t VertexSize() const;

		uint32_t IndexCount() const;
		uint32_t IndexSize() const;

		static bool LoadMeshesFromFile(const std::string& path, std::vector<Mesh>& meshes, std::vector<std::string>* names, bool useIndexBuffer);
	};

	struct Material
	{
		Pipeline* pipeline;
		PipelineLayout* pipelineLayout;
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