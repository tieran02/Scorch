#pragma once
#include "core/app.h"

namespace SC
{
	enum class GraphicsAPI
	{
		VULKAN,
#ifdef RENDERER_D12
		D12,
#endif // RENDERER_D12
		COUNT
	};

	class Pipeline;
	class PipelineLayout;
	class DescriptorSet;
	class Buffer;
	struct Viewport;
	struct Scissor;
	struct Texture;
	class Renderpass;
	struct RenderTarget;
	class CommandBuffer;

	class Renderer
	{
	public:
		static std::unique_ptr<Renderer> Create(GraphicsAPI api);
		virtual ~Renderer();

		GraphicsAPI GetApi() const;

		virtual void Init();
		virtual void Cleanup();
		virtual void CreateSwapchain() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;
		virtual void SubmitCommandBuffer(const CommandBuffer& commandBuffer) = 0;

		virtual CommandBuffer& GetFrameCommandBuffer() const = 0;
		virtual Renderpass* DefaultRenderPass() const = 0;
		virtual RenderTarget* DefaultRenderTarget() const = 0;

		uint8_t FrameDataIndex() const;
		uint8_t FrameDataIndexCount() const;

		Texture* WhiteTexture() const;
		Texture* BlackTexture() const;

		inline bool IsVulkan() const
		{
			return m_api == GraphicsAPI::VULKAN;
		}

		inline bool IsD12() const 
		{
		#ifdef RENDERER_D12
			return m_api == GraphicsAPI::D12;
		#else
			return false;
		#endif // !RENDERER_D12
		}

	protected:
		Renderer(GraphicsAPI api);

		uint32_t m_currentFrame;
	private:
		GraphicsAPI m_api;
	};

	//Number of frame to compute at the same time (Double buffering helps prevent the CPU waiting on the GPU)
	constexpr std::array<uint8_t, to_underlying(GraphicsAPI::COUNT)> FRAME_OVERLAP_COUNT
	{
		3, //Vulkan frame overlap
	};

	//Frame data allows holds data such as buffers/textures for each overlapping frame (Use FrameData when an object gets changes per frame e.g uniforms)
	template<class T>
	struct FrameData
	{
	public:
		template <typename... Args>
		static FrameData<T> Create(Args&&... args);
		uint8_t FrameCount() const { return static_cast<uint8_t>(data.size()); }
		T* GetFrameData(uint8_t index)
		{
			CORE_ASSERT(index >= 0 && index < data.size(), "Invalid frame index");
			return data[index].get();
		}

		std::vector<std::unique_ptr<T>>::iterator begin() { return data.begin(); }
		std::vector<std::unique_ptr<T>>::iterator end() { return data.end(); }
		std::vector<std::unique_ptr<T>>::const_iterator cbegin() { return data.cbegin(); }
		std::vector<std::unique_ptr<T>>::const_iterator cend() { return data.cend(); }

		void ForEach(std::function<void(T*, uint8_t index)> func)
		{
			uint8_t index = 0;
			std::for_each(begin(), end(), [=,&index](std::unique_ptr<T>& ptr)
				{
					func(ptr.get(), index++);
				});
		}

		void reset()
		{
			for (auto& ptr : data)
				ptr.reset();
		}
	private:
		std::vector<std::unique_ptr<T>> data;
	};

	template<class T>
	template <typename... Args>
	static FrameData<T> FrameData<T>::Create(Args&&... args)
	{
		FrameData<T> frameData;

		const App* app = App::Instance();
		CORE_ASSERT(app, "App instance is null");
		if (!app) return frameData;

		const Renderer* renderer = app->GetRenderer();
		CORE_ASSERT(renderer, "renderer is null");
		if (!renderer) return frameData;

		//Create a unique ptr for each frame overlap
		switch (renderer->GetApi())
		{
		case GraphicsAPI::VULKAN:
			frameData.data.resize(FRAME_OVERLAP_COUNT.at(to_underlying(GraphicsAPI::VULKAN)));
		}

		for (int i = 0; i < frameData.data.size(); ++i)
		{
			frameData.data[i] = std::move(T::Create(std::forward<Args>(args)...));
		}

		return frameData;
	}

}