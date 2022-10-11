#include "pch.h"
#include "render/buffer.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanBuffer.h"

using namespace SC;

std::unique_ptr<Buffer> Buffer::Create(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage)
{
	CORE_ASSERT(size > 0, "Size must be greater than 0");
	CORE_ASSERT(bufferUsage.any(), "Buffer usaage must be set");

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
		buffer = std::unique_ptr<VulkanBuffer>(new VulkanBuffer(size, bufferUsage, allocationUsage));
	}

	CORE_ASSERT(buffer, "failed to create buffer");
	return std::move(buffer);
}

Buffer::Buffer(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage) :
	m_size(size),
	m_bufferUsage(bufferUsage),
	m_allocationUsage(allocationUsage)
{

}

Buffer::~Buffer()
{

}

bool Buffer::HasUsage(BufferUsage usage) const
{
	return m_bufferUsage.test(to_underlying(usage));
}

size_t Buffer::GetSize() const
{
	return m_size;
}

ScopedMapData::ScopedMapData(void* mapData, std::function<void()>&& unmapFunc) : 
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

void* const ScopedMapData::Data() const
{
	return m_mapped;
}
