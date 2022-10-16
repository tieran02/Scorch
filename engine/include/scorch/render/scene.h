#pragma once
#include "mesh.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	class Renderer;
	class Buffer;
	class Scene
	{
	public:
		Scene() = default;

		RenderObject* CreateRenderObject(RenderObject&& object);

		Material* CreateMaterial(Pipeline* pipeline, PipelineLayout* pipelineLayout, const std::string& name);
		Material* GetMaterial(const std::string& name);

		Mesh* InsertMesh(const std::string& name, Mesh&& mesh);
		Mesh* GetMesh(const std::string& name);

		void DrawObjects(Renderer* renderer,
			std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc);
	private:
		std::vector<RenderObject> m_renderables;
		std::unordered_map<std::string, Material> m_materials;
		std::unordered_map<std::string, Mesh> m_meshes;

		std::unordered_map<std::string, std::unique_ptr<Buffer>> m_vertexBuffers;
		std::unordered_map<std::string, std::unique_ptr<Buffer>> m_indexBuffers;
	};
}