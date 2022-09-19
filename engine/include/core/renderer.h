#pragma once
namespace SC
{
	enum class GraphicsAPI
	{
		VULKAN
	};

	class Renderer
	{
	public:
		static std::unique_ptr<Renderer> Create(GraphicsAPI api);
		virtual ~Renderer();

		virtual void Init() = 0;
		virtual void Cleanup() = 0;
	protected:
		Renderer() = default;
	};
}