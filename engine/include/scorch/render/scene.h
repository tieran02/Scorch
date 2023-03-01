#pragma once
#include "mesh.h"
#include "materialSystem.h"
#include "scorch/core/sceneGraph.h"

#include "jaam.h"
#include "glm/glm.hpp"
#include "renderer.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	class Buffer;
	struct Texture;

	struct SceneUbo
	{
		glm::vec4 DirectionalLightDir;
		glm::vec4 DirectionalLightColor; //w is intensity
		glm::mat4 ViewMatrix;
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		void DrawObjects(Renderer* renderer,
			std::function<void(const RenderObject& renderObject, bool pipelineChanged)> PerRenderObjectFunc);

		void Reset();

		SceneNode& Root();

		SceneNode* LoadModel(const std::string& path, MaterialSystem* materialSystem);

		SceneUbo& GetSceneData();
		Buffer* GetSceneUniformBuffer(uint8_t frameDataIndex);
	private:
		void CreateSceneUniformBuffers();
		void UpdateSceneUniformBuffers(uint8_t frameDataIndex);

	private:
		SceneNode m_root;

		SceneUbo m_sceneUbo;
		FrameData<Buffer> m_sceneUniformBuffers;

		std::unordered_set<Asset::AssetHandle> m_loadedModels;
		std::unordered_map<std::string, std::shared_ptr<Mesh>> m_meshes;

		std::unordered_set<Asset::AssetHandle> m_loadedMaterial;
		std::unordered_set<Asset::AssetHandle> m_loadedTextures;
	};
}