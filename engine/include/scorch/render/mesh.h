#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>

namespace SC
{
	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec3 color;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;

		uint32_t VertexCount() const;
		uint32_t Size() const;
	};

	struct RenderObject
	{
		Mesh mesh;
		glm::mat4 transform;

		//Load RenderObject from file such as obj/fbx
		static bool LoadFromFile(const std::string& path, std::vector<RenderObject>& model);
	};

}