#include "pch.h"
#include "render/buffer.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanBuffer.h"

using namespace SC;

std::unique_ptr<Buffer> Buffer::Create(size_t size)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<Buffer> buffer{ nullptr };
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		buffer = std::unique_ptr<VulkanBuffer>(new VulkanBuffer(size));
	}

	CORE_ASSERT(buffer, "failed to create buffer");
	return std::move(buffer);
}

Buffer::Buffer(size_t size) : 
	m_size(size)
{

}

Buffer::~Buffer()
{

}

ScopedMapData::ScopedMapData(MappedData mapData, std::function<void()>&& unmapFunc) : 
	m_mapped(mapData),
	m_unmapFunc(unmapFunc)
{
}

ScopedMapData::ScopedMapData() : 
	m_mapped(nullptr)
{

}

ScopedMapData::~ScopedMapData()
{
	m_unmapFunc();
	m_mapped = nullptr;
}

const MappedData ScopedMapData::Data() const
{
	return m_mapped;
}
