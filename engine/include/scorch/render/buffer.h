#pragma once

namespace SC
{
	struct ScopedMapData
	{
	public:
		ScopedMapData();
		ScopedMapData(void* mapData, std::function<void()>&& unmapFunc);
		~ScopedMapData();
		const void* Data() const;
	private:
		void* m_mapped;
		std::function<void()> m_unmapFunc;
	};

	class Buffer
	{
	public:
		static std::unique_ptr<Buffer> Create(size_t size);
		virtual ~Buffer();
		virtual void Destroy() = 0;

		virtual ScopedMapData Map() = 0;
	protected:
		Buffer(size_t size);
		size_t m_size;
	};
}