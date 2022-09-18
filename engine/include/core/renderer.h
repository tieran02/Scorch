#pragma once
enum class GraphicsAPI
{
	VULKAN
};

class Renderer
{
public:
	static std::unique_ptr<Renderer> Create(GraphicsAPI api);

	virtual void Init() = 0;
	virtual void Cleanup() = 0;
protected:
	Renderer() = default;
};
