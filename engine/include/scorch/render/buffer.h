#pragma once

namespace SC
{
	struct ScopedMapData
	{
	public:
		ScopedMapData();
		ScopedMapData(void* mapData, std::function<void()>&& unmapFunc);
		~ScopedMapData();
		void* const Data() const;
	private:
		void* m_mapped;
		std::function<void()> m_unmapFunc;
	};


	enum class BufferUsage
	{
		VERTEX_BUFFER,
		MAP,
		COUNT
	};
	using BufferUsageSet = std::bitset<to_underlying(BufferUsage::COUNT)>;

	enum class AllocationUsage
	{
		HOST,
		DEVICE,
		COUNT
	};

	class Buffer
	{
	public:
		static std::unique_ptr<Buffer> Create(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage);
		virtual ~Buffer();
		virtual void Destroy() = 0;

		virtual ScopedMapData Map() = 0;

		bool HasUsage(BufferUsage usage) const;
	protected:
		Buffer(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage);
		size_t m_size;
		BufferUsageSet m_bufferUsage;
		AllocationUsage m_allocationUsage;
	};
}