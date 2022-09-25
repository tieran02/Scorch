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

		GraphicsAPI GetApi() const;

		virtual void Init() = 0;
		virtual void Cleanup() = 0;

		virtual void Draw() = 0;
	protected:
		Renderer(GraphicsAPI api);
	private:
		GraphicsAPI m_api;
	};
}