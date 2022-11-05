#include "pch.h"
#include "render/materialSystem.h"
#include "render/pipeline.h"

using namespace SC;

ShaderEffect ShaderEffect::Builder(const std::string& vertexShader, const std::string& fragmentShader)
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shaderModule = shaderBuilder.SetVertexModulePath("shaders/textured.vert.spv")
		.SetFragmentModulePath("shaders/textured.frag.spv")
		.Build();

	return ShaderEffect(std::move(shaderModule));;
}

ShaderEffect&& ShaderEffect::AddSet(const std::string& name, std::vector<DescriptorBinding>&& bindings)
{
	CORE_ASSERT(m_usedSetLayouts < 4, "Shader effect can only have 4 sets");
	if (m_usedSetLayouts >= 4) return std::move(*this);

	m_descriptorSetLayouts.at(m_usedSetLayouts++) = DescriptorSetLayout::Create(std::move(bindings));

	return std::move(*this);
}

ShaderEffect&& ShaderEffect::AddPushConstant(const std::string& name, PushConstant&& pushConstant)
{
	m_pushConstants.push_back(std::move(pushConstant));
	return std::move(*this);
}

ShaderEffect&& ShaderEffect::Build()
{
	m_pipelineLayout = SC::PipelineLayout::Create();
	
	for(const auto& pushConstant : m_pushConstants)
	{
		m_pipelineLayout->AddPushConstant(pushConstant.shaderStages, pushConstant.size);
	}
	
	for (const auto& setLayout : m_descriptorSetLayouts)
	{
		if(!setLayout) continue;

		m_pipelineLayout->AddDescriptorSetLayout(setLayout.get());
	}

	m_pipelineLayout->Build();

	return std::move(*this);
}

ShaderEffect::ShaderEffect(std::unique_ptr<ShaderModule>&& shader) :
	m_shaderModule(std::move(shader)),
	m_usedSetLayouts(0)
{

}

ShaderModule* ShaderEffect::GetShaderModule() const
{
	return m_shaderModule.get();
}

DescriptorSetLayout* ShaderEffect::GetDescriptorSetLayout(int index)
{
	CORE_ASSERT(m_usedSetLayouts < 4, "Shader effect can only have 4 sets");
	if (m_usedSetLayouts >= 4) return nullptr;

	return m_descriptorSetLayouts.at(index).get();
}

PipelineLayout* ShaderEffect::GetPipelineLayout()
{
	return m_pipelineLayout.get();
}
