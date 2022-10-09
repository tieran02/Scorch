#pragma once
namespace SC
{
	enum class GraphicsAPI
	{
		VULKAN
	};

	class Pipeline;
	class PipelineLayout;
	class Buffer;
	class Renderer
	{
	public:
		static std::unique_ptr<Renderer> Create(GraphicsAPI api);
		virtual ~Renderer();

		GraphicsAPI GetApi() const;

		virtual void Init() = 0;
		virtual void Cleanup() = 0;

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual void BindPipeline(const Pipeline* pipeline) = 0;
		virtual void BindVertexBuffer(const Buffer* buffer) = 0;
		virtual void BindIndexBuffer(const Buffer* buffer) = 0;
		virtual void PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data) = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

		virtual void Draw() = 0;
	protected:
		Renderer(GraphicsAPI api);
	private:
		GraphicsAPI m_api;
	};
}