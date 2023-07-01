#include "pch.h"
#include "vk/vulkanCommandbuffer.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanInitialiser.h"
#include "vk/vulkanRenderpass.h"
#include "vk/vulkanTexture.h"
#include "vk/vulkanPipeline.h"
#include "vk/vulkanBuffer.h"
#include "render/mesh.h"
#include "../../include/scorch/vk/vulkanDescriptorSet.h"

using namespace SC;

VulkanCommandPool::VulkanCommandPool()
{
	Init();
}

VulkanCommandPool::~VulkanCommandPool()
{
	m_deletionQueue.flush();
}


VkCommandPool VulkanCommandPool::GetCommandPool() const
{
	return m_commandPool;
}

void VulkanCommandPool::Init()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return;

	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(renderer->m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(renderer->m_device, &uploadCommandPoolInfo, nullptr, &m_commandPool));

	m_deletionQueue.push_function([=]() {
		vkDestroyCommandPool(renderer->m_device, m_commandPool, nullptr);
		});
}

std::unique_ptr<SC::CommandBuffer> VulkanCommandPool::CreateCommandBuffer()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer) return nullptr;

	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_commandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(renderer->m_device, &cmdAllocInfo, &commandBuffer));

	DeletionQueue freeCommandBufferQueue;
	freeCommandBufferQueue.push_function([=]() {
		vkFreeCommandBuffers(renderer->m_device, m_commandPool, 1, &commandBuffer);
		});

	std::unique_ptr<VulkanCommandBuffer> vkCommandBuffer = std::make_unique<VulkanCommandBuffer>(std::move(freeCommandBufferQueue));
	vkCommandBuffer->m_commandBuffer = commandBuffer;

	return std::move(vkCommandBuffer);


}

VulkanCommandBuffer::VulkanCommandBuffer(DeletionQueue&& freeCommandQueue) : m_freeCommandQueue(std::move(freeCommandQueue))
{

}

VulkanCommandBuffer::~VulkanCommandBuffer()
{
	m_freeCommandQueue.flush();
}

void VulkanCommandBuffer::BeginRecording()
{
	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(m_commandBuffer, &cmdBeginInfo));
}

void VulkanCommandBuffer::EndRecording()
{
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(m_commandBuffer));
}

void VulkanCommandBuffer::BeginRenderPass(const Renderpass* renderPass, const RenderTarget* renderTarget, float clearR /*= 0*/, float clearG /*= 0*/, float clearB /*= 0*/, float clearDepth /*= 1.0f*/)
{
	VkClearValue clearValue;
	clearValue.color = { { clearR, clearG, clearB, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = clearDepth;

	const VulkanRenderpass* vulkanRenderpass = static_cast<const VulkanRenderpass*>(renderPass);
	const VulkanRenderTarget* vulkanRenderTarget = static_cast<const VulkanRenderTarget*>(renderTarget);

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(vulkanRenderpass->GetRenderPass(),
		VkExtent2D(vulkanRenderTarget->GetWidth(), vulkanRenderTarget->GetHeight()),
		vulkanRenderTarget->m_framebuffer);

	//connect clear values
	rpInfo.clearValueCount = 2;
	VkClearValue clearValues[] = { clearValue, depthClear };
	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(m_commandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::EndRenderPass()
{
	//finalize the render pass
	vkCmdEndRenderPass(m_commandBuffer);
}

void VulkanCommandBuffer::SetViewport(const Viewport& viewport)
{
	//VkViewport and viewport use the same memory layout so just reinterpret_cast
	const VkViewport& vkViewport = reinterpret_cast<const VkViewport&>(viewport);
	vkCmdSetViewport(m_commandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandBuffer::SetScissor(const Scissor& scissor)
{
	//VkRect2D and Scissor use the same memory layout so just reinterpret_cast
	const VkRect2D& vkScissor = reinterpret_cast<const VkRect2D&>(scissor);
	vkCmdSetScissor(m_commandBuffer, 0, 1, &vkScissor);
}

void VulkanCommandBuffer::BindPipeline(const Pipeline* pipeline)
{
	CORE_ASSERT(pipeline, "Pipline can't be null");
	vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<const VulkanPipeline*>(pipeline)->GetPipeline());
}

void VulkanCommandBuffer::BindVertexBuffer(const Buffer* buffer)
{
	CORE_ASSERT(buffer, "Buffer can't be null");
	if (!buffer->HasUsage(BufferUsage::VERTEX_BUFFER))
	{
		CORE_ASSERT(false, "Buffer must be a vertex buffer");
		return;
	}

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(m_commandBuffer, 0, 1, static_cast<const VulkanBuffer*>(buffer)->GetBuffer(), &offset);
}

void VulkanCommandBuffer::BindIndexBuffer(const Buffer* buffer)
{
	CORE_ASSERT(buffer, "Buffer can't be null");
	if (!buffer->HasUsage(BufferUsage::INDEX_BUFFER))
	{
		CORE_ASSERT(false, "Buffer must be a index buffer");
		return;
	}

	VkDeviceSize offset = 0;
	vkCmdBindIndexBuffer(m_commandBuffer, *static_cast<const VulkanBuffer*>(buffer)->GetBuffer(), offset, sizeof(VertexIndexType) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet, int set /*= 0*/)
{
	CORE_ASSERT(descriptorSet, "descriptorSet can't be null");

	const VulkanDescriptorSet* vulkanDescriptorSet = static_cast<const VulkanDescriptorSet*>(descriptorSet);
	VkPipelineLayout layout = static_cast<const VulkanPipelineLayout*>(pipelineLayout)->GetPipelineLayout();

	vkCmdBindDescriptorSets(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1, &vulkanDescriptorSet->m_descriptorSet, 0, nullptr);
}

void VulkanCommandBuffer::PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data)
{
	CORE_ASSERT(pipelineLayout, "Pipline layout can't be null");

	VkPipelineLayout layout = static_cast<const VulkanPipelineLayout*>(pipelineLayout)->GetPipelineLayout();
	if (rangeIndex < 0 && rangeIndex >= pipelineLayout->PushConstants().size())
	{
		CORE_ASSERT(false, "Range index out of range");
		return;
	}

	uint32_t shaderStages = 0;
	if (pipelineLayout->PushConstants()[rangeIndex].shaderStages.test(ShaderStage::VERTEX))
		shaderStages |= VK_SHADER_STAGE_VERTEX_BIT;
	if (pipelineLayout->PushConstants()[rangeIndex].shaderStages.test(ShaderStage::FRAGMENT))
		shaderStages |= VK_SHADER_STAGE_FRAGMENT_BIT;

	vkCmdPushConstants(m_commandBuffer, layout, shaderStages, offset, pipelineLayout->PushConstants()[rangeIndex].size, data);
}

void VulkanCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::ResetCommands()
{
	VK_CHECK(vkResetCommandBuffer(m_commandBuffer, 0));
}

VkCommandBuffer& VulkanCommandBuffer::GetCommandBuffer()
{
	return m_commandBuffer;
}
