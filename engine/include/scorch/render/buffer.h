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


	enum class BufferUsage : uint8_t
	{
		VERTEX_BUFFER,
		INDEX_BUFFER,
		UNIFORM_BUFFER,
		MAP,
		TRANSFER_SRC,
		TRANSFER_DST,
		COUNT
	};
	using BufferUsageSet = Flags<BufferUsage>;

	enum class AllocationUsage
	{
		HOST,
		DEVICE,
		COUNT
	};

	class Buffer
	{
	public:
		static std::unique_ptr<Buffer> Create(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage, void* dataPtr = nullptr);
		virtual ~Buffer();
		virtual void Destroy() = 0;

		virtual ScopedMapData Map() = 0;

		virtual void CopyFrom(Buffer* src) = 0;

		bool HasUsage(BufferUsage usage) const;
		size_t GetSize() const;
	protected:
		Buffer(size_t size, const BufferUsageSet& bufferUsage, AllocationUsage allocationUsage, void* dataPtr);
		size_t m_size;
		BufferUsageSet m_bufferUsage;
		AllocationUsage m_allocationUsage;
	};
}