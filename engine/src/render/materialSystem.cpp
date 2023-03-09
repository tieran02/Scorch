#include "pch.h"
#include "render/materialSystem.h"
#include "render/pipeline.h"
#include "render/texture.h"
#include "core/app.h"
#include "render/renderer.h"
#include "render/buffer.h"

using namespace SC;

ShaderEffect::ShaderEffect() : m_textureSetIndex(1)
{

}

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

ShaderEffect&& ShaderEffect::SetTextureSetIndex(uint8_t index)
{
	m_textureSetIndex = index;
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
	m_usedSetLayouts(0),
	m_textureSetIndex(0)
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

uint8_t ShaderEffect::GetTextureSetIndex() const 
{
	return m_textureSetIndex;
}

void ShaderPass::Build(const ShaderEffect& effect)
{
	if(m_pipeline)
		Log::PrintCore("ShaderPass::Build: Shader pass already built, overwriting", LogSeverity::LogWarning);

	CORE_ASSERT(effect.GetPipelineLayout(), "Shader effect layout can't be null");
	if (m_pipeline || !effect.GetPipelineLayout()) return;

	m_effect = &effect;

	m_pipeline = SC::Pipeline::Create(*m_effect->GetShaderModule());
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //pos
	m_pipeline->vertexInputDescription.PushBackAttribute(SC::Format::R32G32B32_SFLOAT); //normal
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

std::shared_ptr<SC::Material> MaterialSystem::BuildMaterial(const std::string& materialName, const MaterialData& info)
{
	std::shared_ptr<Material> mat;
	//search material in the cache first in case its already built
	auto it = m_materialCache.find(info);
	if (it != m_materialCache.end())
	{
		mat = (*it).second;
		m_materials[materialName] = mat;
	}
	else 
	{

		//need to build the material
		auto newMat = std::make_shared<Material>();
		newMat->original = &m_templateCache[info.baseTemplate];
		//not handled yet
		newMat->passSets[MeshpassType::DirectionalShadow].reset();
		newMat->textures = info.textures;

		//Also build ubos for the user data params
		if (!info.shaderParameters.empty()) 
		{
			for (const auto& param : info.shaderParameters)
			{
				switch (param.second)
				{
				case ShaderParamterTypes::FLOAT:
					//Todo pass in default values from mat info
					newMat->parameters.Register(param.first, 0.0f);
					break;
				}
			}
			newMat->parameters.Finalise();
		}

		//if (!info.textures.empty()) 
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
					forwardLayout = effect->GetDescriptorSetLayout(effect->GetTextureSetIndex());
			}
			if (transparancyPass)
			{
				if (auto effect = transparancyPass->GetShaderEffect())
					transparancyLayout = effect->GetDescriptorSetLayout(effect->GetTextureSetIndex());
			}


			if (forwardLayout)
			{
				auto& forwardDescriptor = newMat->passSets[MeshpassType::Forward];
				forwardDescriptor = forwardDescriptor.Create(forwardLayout);

				for (int i = 0; i < forwardLayout->Bindings().size(); ++i)
				{
					//Only set texture if the type is a sampler
					if (forwardLayout->Bindings()[i].type != DescriptorBindingType::SAMPLER)
					{
						forwardDescriptor.ForEach([=](DescriptorSet* set, uint8_t index)
							{ set->SetBuffer(newMat->parameters.GetBuffer(index), i); });

						continue;
					}

					if (i < info.textures.size())
						forwardDescriptor.ForEach([=](DescriptorSet* set, uint8_t index)
							{ set->SetTexture(info.textures[i], i); });
					else
						forwardDescriptor.ForEach([=](DescriptorSet* set, uint8_t index)
							{ set->SetTexture(App::Instance()->GetRenderer()->WhiteTexture(), i); });
				}

			}

			if (transparancyLayout)
			{
				auto& transparancyDescriptor = newMat->passSets[MeshpassType::Transparency];
				transparancyDescriptor = transparancyDescriptor.Create(transparancyLayout); 

				for (int i = 0; i < transparancyLayout->Bindings().size(); ++i)
				{
					if (i < info.textures.size())
						transparancyDescriptor.ForEach([=](DescriptorSet* set, uint8_t index)
							{ set->SetTexture(info.textures[i], i); });
					else
						transparancyDescriptor.ForEach([=](DescriptorSet* set, uint8_t index)
							{ set->SetTexture(App::Instance()->GetRenderer()->WhiteTexture(), i); });
				}
			}
		}

		Log::Print(string_format("Built New Material {0}", materialName));
		//add material to cache
		m_materialCache[info] = (newMat);
		mat = newMat;
		m_materials[materialName] = mat;
	}

	return mat;
}

std::shared_ptr<SC::Material> MaterialSystem::GetMaterial(const std::string& materialName)
{
	auto it = m_materials.find(materialName);
	if (it != m_materials.end())
	{
		return(*it).second;
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

const std::unordered_map<std::string, std::shared_ptr<SC::Material>>& MaterialSystem::Materials() const
{
	return m_materials;
}

ShaderParameters::ShaderParameters() : m_created(false)
{

}

namespace
{
	template <typename T>
	inline std::vector<uint8_t> DataToVector(T value)
	{
		std::vector<uint8_t> data(sizeof(T));
		memcpy(&value, data.data(), sizeof(T));
		return std::move(data);
	}
}

void ShaderParameters::Register(const std::string& key, float value /*= 0.0f*/)
{
	if (!IsValid(false, true, "", key)) return;
	
	m_defaultData.emplace(key, DataToVector(value));
	m_size += sizeof(value);

	//m_data.resize(m_data.size() + sizeof(float));
	//uint8_t* src = &m_data[m_data.size() - sizeof(float)];

	//m_register[key] = std::make_pair(ShaderParamterTypes::FLOAT, src);

	//float& ref = *static_cast<float*>(m_register[key].second);
	//ref = value;
}

void ShaderParameters::Register(const std::string& key, int value /*= 0*/)
{
	if (!IsValid(false, true, "", key)) return;

	m_data.resize(m_data.size() + sizeof(int));
	uint8_t* src = &m_data[m_data.size() - sizeof(int)];

	m_register[key] = std::make_pair(ShaderParamterTypes::INT, src);
	*reinterpret_cast<int*>(m_register[key].second) = value;
}

void ShaderParameters::Register(const std::string& key, const glm::vec3& value /*= glm::vec3(0)*/)
{
	if (!IsValid(false, true, "", key)) return;

	m_data.resize(m_data.size() + sizeof(glm::vec3));
	uint8_t* src = &m_data[m_data.size() - sizeof(glm::vec3)];

	m_register[key] = std::make_pair(ShaderParamterTypes::VEC3, src);
	*reinterpret_cast<glm::vec3*>(m_register[key].second) = value;
}

void ShaderParameters::Register(const std::string& key, const glm::vec4& value /*= glm::vec4(0)*/)
{
	if(!IsValid(false, true, "", key)) return;

	m_data.resize(m_data.size() + sizeof(glm::vec4));
	uint8_t* src = &m_data[m_data.size() - sizeof(glm::vec4)];

	m_register[key] = std::make_pair(ShaderParamterTypes::VEC4, src);
	*reinterpret_cast<glm::vec4*>(m_register[key].second) = value;
}

void* ShaderParameters::GetAddress(const std::string& key)
{
	if (!IsValid(true, false, key)) return nullptr;
	return m_register.at(key).second;	
}

void ShaderParameters::Set(const std::string& key, float value)
{
	if (!IsValid(true, false, key)) return;

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::FLOAT, "Type is not a float");

	float& ref = *static_cast<float*>(it->second.second);
	ref = value;
}

void ShaderParameters::Set(const std::string& key, int value)
{
	if (!IsValid(true, false, key)) return;

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::INT, "Type is not a int");

	int& ref = *static_cast<int*>(it->second.second);
	ref = value;
}

void ShaderParameters::Set(const std::string& key, const glm::vec3& value /*= glm::vec3(0)*/)
{
	if (!IsValid(true, false, key)) return;

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::VEC3, "Type is not a vec3");

	glm::vec3& ref = *static_cast<glm::vec3*>(it->second.second);
	ref = value;
}

void ShaderParameters::Set(const std::string& key, const glm::vec4& value /*= glm::vec4(0)*/)
{
	if (!IsValid(true, false, key)) return;

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::VEC4, "Type is not a vec4");

	glm::vec4& ref = *static_cast<glm::vec4*>(it->second.second);
	ref = value;
}

float ShaderParameters::GetFloat(const std::string& key)
{
	if (!IsValid(true, false, key)) return 0.0f;

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::FLOAT, "Type is not a float");
	return *static_cast<float*>(it->second.second);
}

int ShaderParameters::GetInt(const std::string& key)
{
	if (!IsValid(true, false, key)) return 0;

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::INT, "Type is not a int");
	return *static_cast<int*>(it->second.second);
}

glm::vec3 ShaderParameters::GetVec3(const std::string& key)
{
	if (!IsValid(true, false, key)) return glm::vec3(0);

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::VEC3, "Type is not a int");
	return *static_cast<glm::vec3*>(it->second.second);
}

glm::vec4 ShaderParameters::GetVec4(const std::string& key)
{
	if (!IsValid(true, false, key)) return glm::vec4(0);

	auto it = m_register.find(key);
	CORE_ASSERT(it->second.first == ShaderParamterTypes::VEC4, "Type is not a int");
	return *static_cast<glm::vec4*>(it->second.second);
}

const std::vector<uint8_t>& ShaderParameters::GetData() const
{
	return m_data;
}

void ShaderParameters::CreateBuffers()
{
	if (!IsValid(false, true)) return;

	BufferUsageSet uboUsage;
	uboUsage.set(BufferUsage::UNIFORM_BUFFER);
	uboUsage.set(BufferUsage::MAP);
	const auto& matData = m_data;
	m_parameterBuffers = SC::FrameData<SC::Buffer>::Create(matData.size(), uboUsage, SC::AllocationUsage::HOST);

	m_parameterBuffers.ForEach([&matData](Buffer* buffer, uint8_t index)
		{
			auto mapped = buffer->Map();
			memcpy(mapped.Data(), matData.data(), matData.size());
		});

	m_created = true;
}

void ShaderParameters::Update(uint8_t frameIndex)
{
	if (!IsValid(true, false)) return;

	auto mapped = m_parameterBuffers.GetFrameData(frameIndex)->Map();
	memcpy(mapped.Data(), m_data.data(), m_data.size());
}

void ShaderParameters::UpdateAll()
{
	if (!IsValid(true, false)) return;

	const auto& matData = m_data;
	m_parameterBuffers.ForEach([&matData](Buffer* buffer, uint8_t index)
		{
			auto mapped = buffer->Map();
			memcpy(mapped.Data(), matData.data(), matData.size());
		});
}

Buffer* ShaderParameters::GetBuffer(uint8_t frameIndex)
{
	return m_parameterBuffers.GetFrameData(frameIndex);
}

bool ShaderParameters::IsValid(bool validateCreated, bool validateNotCreated,
	const std::string& checkRegistered, const std::string& checkNotRegistered)
{
	if (validateCreated)
	{
		CORE_ASSERT(m_created, "ShaderParameters not created");
		if (!m_created) return false;
	}

	if (validateNotCreated)
	{
		CORE_ASSERT(!m_created, "ShaderParameters already created");
		if (m_created) return false;
	}

	if (!checkRegistered.empty())
	{
		auto it = m_register.find(checkRegistered);
		const bool registered = it != m_register.end();
		CORE_ASSERT(registered, "not registered");
		if (!registered) return false;
	}

	if (!checkNotRegistered.empty())
	{
		auto it = m_register.find(checkNotRegistered);
		const bool registered = it != m_register.end();
		CORE_ASSERT(!registered, "already registered");
		if (registered) return false;
	}

	return true;
}

//Need to call finalize after adding all the shader parameters.
//This ensures the data vector is resized and all pointers are valid
void ShaderParameters::Finalise()
{
	if (!IsValid(false, true)) return;

	m_data.resize(m_size);

	//Copy default data into the data vector
	uint8_t* dst = m_data.data();
	for (const auto& defaultData : m_defaultData)
	{
		memcpy(dst, defaultData.second.data(), defaultData.second.size());
		m_register[defaultData.first] = std::make_pair(ShaderParamterTypes::FLOAT, dst);
		dst += defaultData.second.size();
	}

	CreateBuffers();
}

const std::unordered_map<std::string, std::pair<SC::ShaderParamterTypes, void*>>& ShaderParameters::GetRegister() const
{
	return m_register;
}
