#pragma once
#include "mesh.h"
#include "materialSystem.h"
#include "scorch/core/sceneGraph.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	class Renderer;
	class Buffer;
	struct Texture;
	class Scene
	{
	public:
		Scene() = default;

		RenderObject* CreateRenderObject(RenderObject&& object);

		Mesh& InsertMesh(const std::string& name);
		Mesh* GetMesh(const std::string& name);

		Texture* CreateTexture(const std::string& path);

		void DrawObjects(Renderer* renderer,
			std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc);

		void Reset();

		SceneNode& Root();
	private:
		//std::vector<RenderObject> m_renderables;
		SceneNode m_root;

		std::unordered_map<std::string, Mesh> m_meshes;
		std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;

		std::unordered_map<std::string, std::unique_ptr<Buffer>> m_vertexBuffers;
		std::unordered_map<std::string, std::unique_ptr<Buffer>> m_indexBuffers;
	};
}