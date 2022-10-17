#include "render/buffer.h"
#include "volk.h"
#include "vk_mem_alloc.h"
#include "core/utils.h"

namespace SC
{
	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage, void* dataPtr);
		~VulkanBuffer();
		ScopedMapData Map() override;
		void CopyFrom(Buffer* src) override;

		void Destroy() override;

		const VkBuffer* GetBuffer() const;
	private:
		VkBuffer m_buffer;
		VmaAllocation m_allocation;

		DeletionQueue m_deletionQueue;
	};
}