#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include "descriptorSet.h"
#include "shaderModule.h"
#include "pipeline.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;

	struct ShaderEffect
	{
	public:
		ShaderEffect() = default;

		static ShaderEffect Builder(const std::string& vertexShader, const std::string& fragmentShader);
		ShaderEffect&& AddSet(const std::string& name, std::vector<DescriptorBinding>&& bindings);
		ShaderEffect&& AddPushConstant(const std::string& name, PushConstant&& pushConstant);
		ShaderEffect&& Build();
	public:
		ShaderModule* GetShaderModule() const;
		DescriptorSetLayout* GetDescriptorSetLayout(int index) const;
		PipelineLayout* GetPipelineLayout() const;
	private:
		ShaderEffect(std::unique_ptr<ShaderModule>&& shader);

		std::unique_ptr<ShaderModule> m_shaderModule;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;

		std::array<std::unique_ptr<DescriptorSetLayout>, 4> m_descriptorSetLayouts;
		std::vector<PushConstant> m_pushConstants;

		uint8_t m_usedSetLayouts;
	};

	struct ShaderPass
	{
		void Build(const ShaderEffect& effect);
		const ShaderEffect* GetShaderEffect() const;
		Pipeline* GetPipeline() const;

	private:
		const ShaderEffect* m_effect{ nullptr };
		std::unique_ptr<Pipeline> m_pipeline{ nullptr };
	};

	enum class MeshpassType 
	{
		Forward,
		Transparency,
		DirectionalShadow
	};

	template<typename T>
	struct PerPassData 
	{
	public:
		T& operator[](MeshpassType pass)
		{
			switch (pass)
			{
			case MeshpassType::Forward:
				return data[0];
			case MeshpassType::Transparency:
				return data[1];
			case MeshpassType::DirectionalShadow:
				return data[2];
			}
			assert(false);
			return data[0];
		};

		void clear(T&& val)
		{
			for (int i = 0; i < 3; i++)
			{
				data[i] = val;
			}
		}

	private:
		std::array<T, 3> data;
	};

	enum class TransparencyMode :uint8_t {
		Opaque,
		Transparent,
		Masked
	};

	struct EffectTemplate 
	{
		EffectTemplate();
		PerPassData<ShaderPass*> passShaders;
		TransparencyMode transparency;
	};

	struct Material {
		EffectTemplate* original;
		PerPassData<std::unique_ptr<DescriptorSet>> passSets;
		std::vector<Texture*> textures; //Material doesn't own textures
	};

	struct MaterialInfo
	{
		std::string baseEffect;
		std::string materialName;
		std::vector<std::string> textures; //path
		TransparencyMode transparency;
	};

	struct MaterialData 
	{
		std::vector<Texture*> textures; //Material doesn't own textures
		std::string baseTemplate;

		bool operator==(const MaterialData& other) const;
		size_t hash() const;
	};

	class MaterialSystem
	{
	public:
		EffectTemplate* AddEffectTemplate(const std::string& name, const EffectTemplate& effectTemplate);
		Material* BuildMaterial(const std::string& materialName, const MaterialData& info);
		Material* GetMaterial(const std::string& materialName);
	private:
		struct MaterialInfoHash
		{
			std::size_t operator()(const MaterialData& k) const
			{
				return k.hash();
			}
		};
	private:
		std::unordered_map<std::string, EffectTemplate> m_templateCache;
		std::unordered_map<std::string, std::shared_ptr<Material>> m_materials;
		std::unordered_map<MaterialData, std::shared_ptr<Material>, MaterialInfoHash> m_materialCache;
	};

}