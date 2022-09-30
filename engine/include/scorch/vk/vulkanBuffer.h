#include "render/buffer.h"
#include "volk.h"
#include "vk_mem_alloc.h"

namespace SC
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage);
		~VulkanBuffer() override;
		ScopedMapData Map() override;

		void Destroy() override;

	private:
		VkBuffer m_buffer;
		VmaAllocation m_allocation;
	};
}