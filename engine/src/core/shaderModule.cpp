#include "pch.h"
#include "core/shaderModule.h"
#include "core/renderer.h"

using namespace SC;
ShaderModuleBuilder& ShaderModuleBuilder::SetVertexModulePath(const std::string& path)
{
	m_vertexModulePath = path;
	return *this;
}

ShaderModuleBuilder& ShaderModuleBuilder::SetFragmentModulePath(const std::string& path)
{
	m_fragmentModulePath = path;
	return *this;
}

std::unique_ptr<ShaderModule> ShaderModuleBuilder::Build()
{
	std::unique_ptr<ShaderModule> shader = std::unique_ptr<ShaderModule>(new ShaderModule());
	
	//TODO multithread reads
	bool result = false;
	if (!m_vertexModulePath.empty())
		result = shader->LoadModule(ShaderModule::Stage::VERTEX, m_vertexModulePath);
	CORE_ASSERT(result, string_format("Failed to load vertex module: %s", m_vertexModulePath.c_str()));

	if (!m_fragmentModulePath.empty())
		result = shader->LoadModule(ShaderModule::Stage::FRAGMENT, m_fragmentModulePath);
	CORE_ASSERT(result, string_format("Failed to load fragment module: %s", m_fragmentModulePath.c_str()));

	return std::move(shader);
}

ShaderModule::ShaderModule()
{

}

bool ShaderModule::LoadModule(Stage stage, const std::string& modulePath)
{
	//check if module not already loaded
	CORE_ASSERT(m_modules.at(to_underlying(stage)).empty(), "Module already exists");
	if (!m_modules.at(to_underlying(stage)).empty())
		return false;

	//open the file. With cursor at the end
	std::ifstream file(modulePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) 
	{
		return false;
	}

	//find what the size of the file is by looking up the location of the cursor
	//because the cursor is at the end, it gives the size directly in bytes
	size_t fileSize = (size_t)file.tellg();

	//spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
	auto& buffer = m_modules.at(to_underlying(stage));
	buffer.resize(fileSize / sizeof(uint32_t));
	
	//put file cursor at beginning
	file.seekg(0);

	//load the entire file into the buffer
	file.read((char*)buffer.data(), fileSize);

	//now that the file is loaded into the buffer, we can close it
	file.close();
}
