#pragma once
enum class GraphicsAPI
{
	VULKAN
};

class Renderer
{
public:
	static std::unique_ptr<Renderer> Create(GraphicsAPI api);
protected:
	Renderer() = default;
};
