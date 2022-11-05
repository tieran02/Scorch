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

	struct MaterialData
	{
		std::string materialName;
		std::string diffuseTexturePath;
	};

	struct Material
	{
		Pipeline* pipeline;
		PipelineLayout* pipelineLayout;
		std::unique_ptr<DescriptorSet> textureDescriptorSet;
	};

	/*class MaterialManager
	{
		void CreateMaterial(const std::string name, )
	private:
		std::unordered_map<std::string, Material> m_materials;
	};*/

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
		DescriptorSetLayout* GetDescriptorSetLayout(int index);
		PipelineLayout* GetPipelineLayout();
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
		ShaderEffect* effect{ nullptr };
		Pipeline* pipeline{ nullptr };
		PipelineLayout* layout{ nullptr };
	};
}