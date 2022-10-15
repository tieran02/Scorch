#pragma once

namespace SC
{
	enum class ShaderStage : uint8_t
	{
		VERTEX,
		FRAGMENT,
		COUNT
	};

	using ShaderModuleFlags = Flags<ShaderStage>;
	template <typename T>
	using ShaderModuleArray = std::array<T, to_underlying(ShaderStage::COUNT)>;
	using ShaderBufferType = std::vector<uint32_t>;

	class ShaderModule
	{
		friend struct ShaderModuleBuilder;
	public:
		bool LoadModule(ShaderStage stage, const std::string& modulePath);
		const ShaderBufferType& GetModule(ShaderStage stage) const;
	private:
		ShaderModule();
		ShaderModuleArray<ShaderBufferType> m_modules;
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
