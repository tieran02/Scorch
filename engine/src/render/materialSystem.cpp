#include "pch.h"
#include "render/materialSystem.h"
#include "render/pipeline.h"
#include "render/texture.h"
#include "core/app.h"
#include "render/renderer.h"

using namespace SC;

ShaderEffect ShaderEffect::Builder(const std::string& vertexShader, const std::string& fragmentShader)
{
	SC::ShaderModuleBuilder shaderBuilder;
	auto shaderModule = shaderBuilder.SetVertexModulePath(vertexShader)
		.SetFragmentModulePath(fragmentShader)
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

DescriptorSetLayout* ShaderEffect::GetDescriptorSetLayout(int index) const
{
	CORE_ASSERT(m_usedSetLayouts < 4, "Shader effect can only have 4 sets");
	if (m_usedSetLayouts >= 4) return nullptr;

	return m_descriptorSetLayouts.at(index).get();
}

PipelineLayout* ShaderEffect::GetPipelineLayout() const
{
	return m_pipelineLayout.get();
}

void ShaderPass::Build(const ShaderEffect& effect)
{
	CORE_ASSERT(!m_pipeline, "Shader pass already built");
	CORE_ASSERT(effect.GetPipelineLayout(), "Shader effect layout can't be null");
	if (m_pipeline || !effect.GetPipelineLayout()) return;

	m_effect = &effect;

	m_pipeline = SC::Pipeline::Create(*m_effect->GetShaderModule());
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //pos
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //normal
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //color
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32_SFLOAT); //uvS
	m_pipeline->pipelineLayout = m_effect->GetPipelineLayout();
	m_pipeline->Build();
}

const ShaderEffect* ShaderPass::GetShaderEffect() const
{
	return m_effect;
}

Pipeline* ShaderPass::GetPipeline() const
{
	return m_pipeline.get();
}

EffectTemplate::EffectTemplate() :
	passShaders(nullptr),
	transparency(TransparencyMode::Opaque)
{
}


bool MaterialData::operator==(const MaterialData& other) const
{
	if (other.baseTemplate.compare(baseTemplate) != 0 || other.textures.size() != textures.size())
	{
		return false;
	}
	else {
		//binary compare textures
		bool comp = memcmp(other.textures.data(), textures.data(), textures.size() * sizeof(textures[0])) == 0;
		return comp;
	}
}

size_t MaterialData::hash() const
{
	using std::size_t;
	using std::hash;

	size_t result = hash<std::string>()(baseTemplate);

	for (const auto& b : textures)
	{
		//pack the binding data into a single int64. Not fully correct but its ok
		size_t texture_hash = (std::hash<size_t>()((size_t)b) << 3) & (std::hash<size_t>()((size_t)b->GetFormat()) >> 7);

		//shuffle the packed binding data and xor it with the main hash
		result ^= std::hash<size_t>()(texture_hash);
	}

	return result;
}

Material* MaterialSystem::BuildMaterial(const std::string& materialName, const MaterialData& info)
{
	std::shared_ptr<Material> mat;
	//search material in the cache first in case its already built
	auto it = m_materialCache.find(info);
	if (it != m_materialCache.end())
	{
		mat = (*it).second;
		m_materials[materialName] = mat;
	}
	else {

		//need to build the material
		auto newMat = std::make_shared<Material>();
		newMat->original = &m_templateCache[info.baseTemplate];
		//not handled yet
		newMat->passSets[MeshpassType::DirectionalShadow] = nullptr;
		newMat->textures = info.textures;

		if (!info.textures.empty()) 
		{
			ShaderPass* forwadPass = newMat->original->passShaders[MeshpassType::Forward];
			ShaderPass* transparancyPass = newMat->original->passShaders[MeshpassType::Transparency];

			CORE_ASSERT(forwadPass || transparancyPass, "pass shaders must be set");
			if (!forwadPass && !transparancyPass) return nullptr;

			DescriptorSetLayout* forwardLayout{ nullptr };
			DescriptorSetLayout* transparancyLayout{ nullptr };
			if (forwadPass)
			{
				if (auto effect = forwadPass->GetShaderEffect())
					forwardLayout = effect->GetDescriptorSetLayout(1);
			}
			if (transparancyPass)
			{
				if (auto effect = transparancyPass->GetShaderEffect())
					transparancyLayout = effect->GetDescriptorSetLayout(1);
			}


			if (forwardLayout)
			{
				auto& forwardDescriptor = newMat->passSets[MeshpassType::Forward];
				forwardDescriptor = std::move(DescriptorSet::Create(forwardLayout));

				for (int i = 0; i < forwardLayout->Bindings().size(); ++i)
				{
					if (i < info.textures.size())
						forwardDescriptor->SetTexture(info.textures[i], i);
					else
						forwardDescriptor->SetTexture(App::Instance()->GetRenderer()->WhiteTexture(), i); //Fill empty textures 
				}

			}

			if (transparancyLayout)
			{
				auto& transparancyDescriptor = newMat->passSets[MeshpassType::Transparency];
				transparancyDescriptor = std::move(DescriptorSet::Create(transparancyLayout));

				for (int i = 0; i < transparancyLayout->Bindings().size(); ++i)
				{
					if (i < info.textures.size())
						transparancyDescriptor->SetTexture(info.textures[i], i);
					else
						transparancyDescriptor->SetTexture(App::Instance()->GetRenderer()->WhiteTexture(), i); //Fill empty textures 
				}
			}
		}

		Log::Print(string_format("Built New Material %s", materialName));
		//add material to cache
		m_materialCache[info] = (newMat);
		mat = newMat;
		m_materials[materialName] = mat;
	}

	return mat.get();
}

SC::Material* MaterialSystem::GetMaterial(const std::string& materialName)
{
	auto it = m_materials.find(materialName);
	if (it != m_materials.end())
	{
		return(*it).second.get();
	}
	else {
		return nullptr;
	}
}

SC::EffectTemplate* MaterialSystem::AddEffectTemplate(const std::string& name, const EffectTemplate& effectTemplate)
{
	auto it = m_templateCache.find(name);
	if (it != m_templateCache.end())
	{
		return &(*it).second;
	}
	else
	{
		m_templateCache[name] = effectTemplate;
		return &m_templateCache[name];
	}
}

