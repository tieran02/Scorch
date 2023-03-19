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

	static constexpr uint32_t MAX_LIGHTS = 8;
	struct Light
	{
		glm::vec4 position;	//w == 0 pointlight
		glm::vec4 intensities; //w is intensity
	};

	struct SceneUbo
	{
		alignas(16) glm::mat4 ViewMatrix;
		alignas(16) glm::vec4 EyePos;
		alignas(4) uint32_t LightCount;
		alignas(16) Light Lights[MAX_LIGHTS];
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