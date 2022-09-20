#pragma once

namespace SC
{
	class ShaderModule
	{
		friend struct ShaderModuleBuilder;
	public:
		enum class Stage
		{
			VERTEX,
			FRAGMENT,
			COUNT
		};

		bool LoadModule(Stage stage, const std::string& modulePath);
	private:
		ShaderModule();
		std::array<std::vector<uint32_t>, to_underlying(Stage::COUNT)> m_modules;
	};

	struct ShaderModuleBuilder
	{
	public:
		ShaderModuleBuilder& SetVertexModulePath(const std::string& path);
		ShaderModuleBuilder& SetFragmentModulePath(const std::string& path);
		std::unique_ptr<ShaderModule> Build();
	private:
		std::string m_vertexModulePath;
		std::string m_fragmentModulePath;
	};
}
