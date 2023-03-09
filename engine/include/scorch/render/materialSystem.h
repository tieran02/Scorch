#pragma once
#include <glm/vec3.hpp>
#include <glm/gtx/transform.hpp>
#include "descriptorSet.h"
#include "shaderModule.h"
#include "pipeline.h"
#include "renderer.h"

namespace SC
{
	class Pipeline;
	class PipelineLayout;
	class Buffer;

	enum class ShaderParamterTypes
	{
		FLOAT,
		INT,
		VEC3,
		VEC4,
	};

	//ShaderParameters creates a contiguous memory block from the register data in the order it was registered
	//E.G 
	//	Register Float
	//	Register Int
	//	Register Vec3
	//Will result in a packed memory block of a float->int->Vec3
	//This can then be uploaded to the GPU as a uniform
	struct ShaderParameters
	{
		ShaderParameters();

		void Update(uint8_t frameIndex);
		void UpdateAll(); //Caution: This may update a buffer that is in flight

		void Register(const std::string& key, float defaultValue = 0.0f);
		void Register(const std::string& key, int defaultValue = 0);
		void Register(const std::string& key, const glm::vec3& defaultValue = glm::vec3(0));
		void Register(const std::string& key, const glm::vec4& defaultValue = glm::vec4(0));
		void Finalise();

		void* GetAddress(const std::string& key);

		void Set(const std::string& key, float value);
		void Set(const std::string& key, int value);
		void Set(const std::string& key, const glm::vec3& value = glm::vec3(0));
		void Set(const std::string& key, const glm::vec4& value = glm::vec4(0));

		float GetFloat(const std::string& key);
		int GetInt(const std::string& key);
		glm::vec3 GetVec3(const std::string& key);
		glm::vec4 GetVec4(const std::string& key);

		const std::vector<uint8_t>& GetData() const;
		Buffer* GetBuffer(uint8_t frameIndex);
	private:
		std::unordered_map<std::string, std::pair<ShaderParamterTypes, void*>> m_register;
		std::vector<uint8_t> m_data;
		std::unordered_map<std::string,std::vector<uint8_t>> m_defaultData;

		size_t m_size;
		bool m_created;

		void CreateBuffers();
		bool IsValid(bool validateCreated, bool validateNotCreated,
			const std::string& checkRegistered = "", const std::string& checkNotRegistered = "");

		FrameData<Buffer> m_parameterBuffers;
	};

	struct ShaderEffect
	{
	public:
		ShaderEffect();

		static ShaderEffect Builder(const std::string& vertexShader, const std::string& fragmentShader);
		ShaderEffect&& AddSet(const std::string& name, std::vector<DescriptorBinding>&& bindings);
		ShaderEffect&& AddPushConstant(const std::string& name, PushConstant&& pushConstant);
		ShaderEffect&& SetTextureSetIndex(uint8_t index);
		ShaderEffect&& Build();
	public:
		ShaderModule* GetShaderModule() const;
		DescriptorSetLayout* GetDescriptorSetLayout(int index) const;
		PipelineLayout* GetPipelineLayout() const;
		uint8_t GetTextureSetIndex() const;
	private:
		ShaderEffect(std::unique_ptr<ShaderModule>&& shader);

		std::unique_ptr<ShaderModule> m_shaderModule;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;

		std::array<std::unique_ptr<DescriptorSetLayout>, 4> m_descriptorSetLayouts;
		std::vector<PushConstant> m_pushConstants;

		uint8_t m_usedSetLayouts;
		uint8_t m_textureSetIndex;
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
		PerPassData() = default;
		PerPassData(T&& val)
		{
			clear(std::move(val));
		}

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

	struct Material
	{
		EffectTemplate* original;
		PerPassData<FrameData<DescriptorSet>> passSets;
		std::vector<Texture*> textures; //Material doesn't own textures

		ShaderParameters parameters;
	};

	struct MaterialData 
	{
		std::vector<Texture*> textures; //Material doesn't own textures
		std::vector<std::pair<std::string, ShaderParamterTypes>> shaderParameters;
		std::string baseTemplate;

		bool operator==(const MaterialData& other) const;
		size_t hash() const;
	};

	class MaterialSystem
	{
	public:
		EffectTemplate* AddEffectTemplate(const std::string& name, const EffectTemplate& effectTemplate);
		std::shared_ptr<Material> BuildMaterial(const std::string& materialName, const MaterialData& info);
		std::shared_ptr<Material> GetMaterial(const std::string& materialName);
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