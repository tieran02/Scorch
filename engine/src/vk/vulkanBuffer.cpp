#include "pch.h"
#include "vk/vulkanBuffer.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"

using namespace SC;

VulkanBuffer::VulkanBuffer(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage, const void* dataPtr) : Buffer(size, bufferUsage,allocationUsage),
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
	bufferInfo.usage = 0;
	if (m_bufferUsage.test(BufferUsage::VERTEX_BUFFER))
		bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	if (m_bufferUsage.test(BufferUsage::INDEX_BUFFER))
		bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	if (m_bufferUsage.test(BufferUsage::UNIFORM_BUFFER))
		bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	if (m_bufferUsage.test(BufferUsage::TRANSFER_SRC))
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (m_bufferUsage.test(BufferUsage::TRANSFER_DST))
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocInfo = {};
	switch (m_allocationUsage)
	{
	case AllocationUsage::HOST:		
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
		break;
	case AllocationUsage::DEVICE:
		CORE_ASSERT(!m_bufferUsage.test(BufferUsage::MAP), "Must be allocated to host memory for mapping");
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
		bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		break;
	default:
		CORE_ASSERT(false, "Failed to find supported allocation usage (default to auto)")
		allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
		break;
	}

	//If we have the map usage make sure we set VMA to allow mapping to this buffer
	if (m_bufferUsage.test(BufferUsage::MAP))
		allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

	VK_CHECK(vmaCreateBuffer(renderer->m_allocator, &bufferInfo, &allocInfo, &m_buffer, &m_allocation, nullptr));

	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vmaDestroyBuffer(renderer->m_allocator, m_buffer, m_allocation);
		vmaFlushAllocation(renderer->m_allocator, m_allocation, 0, size);

		});

	//upload data if we have set the dataPtr
	if (dataPtr)
	{
		if (m_bufferUsage.test(BufferUsage::MAP))
		{
			auto mappedData = Map();
			memcpy(mappedData.Data(), dataPtr, m_size);
		}
		else if (m_allocationUsage == AllocationUsage::DEVICE)
		{
			//Create a staging buffer and copy data to GPU
			BufferUsageSet stagingUsage = bufferUsage;
			stagingUsage.set(BufferUsage::TRANSFER_SRC);
			stagingUsage.set(BufferUsage::MAP);
			VulkanBuffer stagingBuffer(size, stagingUsage, AllocationUsage::HOST, dataPtr);

			CopyFrom(&stagingBuffer);
		}
	}
}

VulkanBuffer::~VulkanBuffer()
{
	Destroy();
}


void VulkanBuffer::Destroy()
{
	m_deletionQueue.flush();
}

ScopedMapData VulkanBuffer::Map()
{
	if (!HasUsage(BufferUsage::MAP))
	{
		CORE_ASSERT(false, "Buffer must have the MAP usage set in order to map data");
		return ScopedMapData();
	}

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return ScopedMapData();

	void* mapped{ nullptr };
	if (const VulkanRenderer* renderer = app->GetVulkanRenderer())
	{
		vmaMapMemory(renderer->m_allocator, m_allocation, &mapped);
		CORE_ASSERT(mapped, "Failed to map buffer data");

		return ScopedMapData(mapped, [=]() 
			{
				vmaUnmapMemory(renderer->m_allocator, m_allocation);
			});
	}
	CORE_ASSERT(false, "Failed to map buffer data");
	return ScopedMapData();
}

void VulkanBuffer::CopyFrom(Buffer* src)
{
	CORE_ASSERT(src, "src buffer cant be null");
	if (!src) return;

	if (!src->HasUsage(BufferUsage::TRANSFER_SRC)) 
	{
		CORE_ASSERT(false, "Src buffer must have BufferUsage::TRANSFER_SRC");
		return;
	}

	if (!HasUsage(BufferUsage::TRANSFER_DST))
	{
		CORE_ASSERT(false, "Dst buffer must have BufferUsage::TRANSFER_DST");
		return;
	}

	if (src->GetSize() > GetSize())
	{
		CORE_ASSERT(false, "Src is larger than dst");
		return;
	}

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	renderer->ImmediateSubmit([=](VkCommandBuffer cmd) {
		VkBufferCopy copy;
		copy.dstOffset = 0;
		copy.srcOffset = 0;
		copy.size = m_size;
		vkCmdCopyBuffer(cmd, *static_cast<VulkanBuffer*>(src)->GetBuffer(), *GetBuffer(), 1, &copy);
		});
}

const VkBuffer* VulkanBuffer::GetBuffer() const
{
	return &m_buffer;
}
