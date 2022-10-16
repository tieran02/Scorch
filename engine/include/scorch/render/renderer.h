#pragma once
#include "core/app.h"

namespace SC
{
	enum class GraphicsAPI
	{
		VULKAN,
		COUNT
	};

	class Pipeline;
	class PipelineLayout;
	class DescriptorSet;
	class Buffer;
	struct Viewport;
	struct Scissor;
	class Renderer
	{
	public:
		static std::unique_ptr<Renderer> Create(GraphicsAPI api);
		virtual ~Renderer();

		GraphicsAPI GetApi() const;

		virtual void Init() = 0;
		virtual void Cleanup() = 0;
		virtual void CreateSwapchain() = 0;

		virtual void BeginFrame(float clearR = 0, float clearG = 0, float clearB = 0) = 0;
		virtual void EndFrame() = 0;

		virtual void SetViewport(const Viewport& viewport) = 0;
		virtual void SetScissor(const Scissor& viewport) = 0;

		virtual void BindPipeline(const Pipeline* pipeline) = 0;
		virtual void BindVertexBuffer(const Buffer* buffer) = 0;
		virtual void BindIndexBuffer(const Buffer* buffer) = 0;
		virtual void BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet) = 0;
		virtual void PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data) = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

		uint8_t FrameDataIndex() const;
		uint8_t FrameDataIndexCount() const;
	protected:
		Renderer(GraphicsAPI api);

		uint32_t m_currentFrame;
	private:
		GraphicsAPI m_api;
	};

	//Number of frame to compute at the same time (Double buffering helps prevent the CPU waiting on the GPU)
	constexpr std::array<uint8_t, to_underlying(GraphicsAPI::COUNT)> FRAME_OVERLAP_COUNT
	{
		2, //Vulkan frame overlap
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
			CORE_ASSERT(index > 0 && index < data.size(), "Invalid frame index");
			return data[index].get();
		}

		std::vector<std::unique_ptr<T>>::iterator begin() { return data.begin(); }
		std::vector<std::unique_ptr<T>>::iterator end() { return data.end(); }
		std::vector<std::unique_ptr<T>>::const_iterator cbegin() { return data.cbegin(); }
		std::vector<std::unique_ptr<T>>::const_iterator cend() { return data.cend(); }
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