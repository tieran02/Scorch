#pragma once
#include "render/commandbuffer.h"
#include "volk.h"
#include "core/utils.h"


namespace SC
{
	class VulkanCommandPool : public CommandPool
	{
	public:
		VulkanCommandPool();
		~VulkanCommandPool();

		VkCommandPool GetCommandPool() const;

		std::unique_ptr<CommandBuffer> CreateCommandBuffer() override;

	private:
		void Init();
	private:
		VkCommandPool m_commandPool;

		DeletionQueue m_deletionQueue;
	};

	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		friend class VulkanCommandPool;
		VulkanCommandBuffer(DeletionQueue&& freeCommandQueue);
		~VulkanCommandBuffer();

		void BeginRecording();
		void EndRecording();

		void BeginRenderPass(const Renderpass* renderPass, const RenderTarget* renderTarget, float clearR = 0, float clearG = 0, float clearB = 0, float clearDepth = 1.0f);
		void EndRenderPass();


		void SetViewport(const Viewport& viewport);
		void SetScissor(const Scissor& scissor);

		void BindPipeline(const Pipeline* pipeline);
		void BindVertexBuffer(const Buffer* buffer);
		void BindIndexBuffer(const Buffer* buffer);
		void BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet, int set = 0);
		void PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data);
		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);

		void ResetCommands();

		VkCommandBuffer& GetCommandBuffer();
	private:
		VkCommandBuffer m_commandBuffer;
		DeletionQueue m_freeCommandQueue; //Frees the command buffer using the command pool that was used to create this buffer
	};
}