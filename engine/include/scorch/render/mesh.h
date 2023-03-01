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
	struct Transform;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	struct Mesh
	{
	public:
		Mesh();
		~Mesh();
		Mesh(Mesh&& other);

		std::vector<Vertex> vertices;
		std::vector<VertexIndexType> indices;
		std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

		uint32_t VertexCount() const;
		uint32_t VertexSize() const;

		uint32_t IndexCount() const;
		uint32_t IndexSize() const;

		bool Build();

		Mesh& operator=(Mesh&& other);
	};

	struct RenderObject
	{
		RenderObject();

		std::string name;
		Mesh* mesh;
		Material* material;
		Transform* transform;
	};
}