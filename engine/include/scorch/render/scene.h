#pragma once
#include "mesh.h"
#include "materialSystem.h"
#include "scorch/core/sceneGraph.h"

#include "jaam.h"

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
		~Scene();

		void DrawObjects(Renderer* renderer,
			std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc);

		void Reset();

		SceneNode& Root();

		SceneNode* LoadModel(const std::string& path, MaterialSystem* materialSystem);
	private:
		//std::vector<RenderObject> m_renderables;
		SceneNode m_root;

		std::unordered_set<Asset::AssetHandle> m_loadedModels;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;

		std::unordered_set<Asset::AssetHandle> m_loadedMaterial;
		std::unordered_set<Asset::AssetHandle> m_loadedTextures;
	};
}