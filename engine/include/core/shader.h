#pragma once

namespace SC
{
	enum class GraphicsAPI;

	class Shader
	{
	public:
		enum class Stage
		{
			VERTEX,
			FRAGMENT,
			COUNT
		};

		static std::unique_ptr<Shader> Create(GraphicsAPI api);
		bool LoadModule(Stage stage, const std::string& modulePath);
	protected:
		Shader();
		std::array<std::vector<uint32_t>, to_underlying(Stage::COUNT)> m_modules;
		//std::unordered_map<Stage, std::vector<uint32_t>> m_modules;
	};

	struct ShaderBuilder
	{
	public:
		ShaderBuilder& SetVertexModulePath(const std::string& path);
		ShaderBuilder& SetFragmentModulePath(const std::string& path);
		std::unique_ptr<Shader> Build(GraphicsAPI api);
	private:
		std::string m_vertexModulePath;
		std::string m_fragmentModulePath;
	};
}
