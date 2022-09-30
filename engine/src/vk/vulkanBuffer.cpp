#include "pch.h"
#include "vk/vulkanBuffer.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"

using namespace SC;

VulkanBuffer::VulkanBuffer(size_t size) : Buffer(size),
m_buffer(VK_NULL_HANDLE),
m_allocation(VK_NULL_HANDLE)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	vmaCreateBuffer(renderer->m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr);
}

VulkanBuffer::~VulkanBuffer()
{
	Destroy();
}


void VulkanBuffer::Destroy()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	vmaDestroyBuffer(renderer->m_allocator, m_buffer, m_allocation);
}

ScopedMapData VulkanBuffer::Map()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return ScopedMapData();

	void* mapped{ nullptr };
	if (const VulkanRenderer* renderer = app->GetVulkanRenderer())
	{
		vmaMapMemory(renderer->m_allocator, m_allocation, &mapped);

		return ScopedMapData(mapped, [=]() 
			{
				vmaUnmapMemory(renderer->m_allocator, m_allocation);
			});
	}
	CORE_ASSERT(mapped, "Failed to map buffer data");
	return ScopedMapData();
}