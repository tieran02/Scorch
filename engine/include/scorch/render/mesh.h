#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>

namespace SC
{
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
	};

	struct RenderObject
	{
		RenderObject();

		Mesh mesh;
		glm::mat4 transform;

		//Load RenderObject from file such as obj/fbx
		static bool LoadFromFile(const std::string& path, std::vector<RenderObject>& model, bool useIndexBuffer);
	};

}