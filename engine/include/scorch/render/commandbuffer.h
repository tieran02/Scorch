#pragma once

namespace SC
{
	class Renderpass;
	struct RenderTarget;
	struct Viewport;
	struct Scissor;
	class Pipeline;
	class Buffer;
	class PipelineLayout;
	class DescriptorSet;

	class CommandBuffer
	{
	public:
		virtual ~CommandBuffer() = default;

		virtual void BeginRecording() const = 0;
		virtual void EndRecording() const = 0;

		virtual void BeginRenderPass(const Renderpass* renderPass, const RenderTarget* renderTarget, float clearR = 0, float clearG = 0, float clearB = 0, float clearDepth = 1.0f) = 0;
		virtual void EndRenderPass() = 0;


		virtual void SetViewport(const Viewport& viewport) = 0;
		virtual void SetScissor(const Scissor& scissor) = 0;

		virtual void BindPipeline(const Pipeline* pipeline) = 0;
		virtual void BindVertexBuffer(const Buffer* buffer) = 0;
		virtual void BindIndexBuffer(const Buffer* buffer) = 0;
		virtual void BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet, int set = 0) = 0;
		virtual void PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data) = 0;
		virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

		virtual void ResetCommands() = 0;
	};

	class CommandPool
	{
	public:
		static std::unique_ptr<CommandPool> Create();
		virtual ~CommandPool() = default;

		virtual std::unique_ptr<CommandBuffer> CreateCommandBuffer() = 0;
	protected:
		CommandPool() = default;
	};
}