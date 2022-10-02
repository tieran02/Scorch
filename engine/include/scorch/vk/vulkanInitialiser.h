#pragma once
#include "volk.h"

namespace vkinit
{
	VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);

	VkFramebufferCreateInfo FramebufferCreateInfo(VkRenderPass renderPass, VkExtent2D extent);

	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = 0);

	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);

	VkSubmitInfo SubmitInfo(VkCommandBuffer* cmd);

	VkPresentInfoKHR PresentInfo();

	VkRenderPassBeginInfo RenderpassBeginInfo(VkRenderPass renderPass, VkExtent2D windowExtent, VkFramebuffer framebuffer);

	VkPipelineShaderStageCreateInfo PipelineShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo();
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo(VkPrimitiveTopology topology);
	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode polygonMode);
	VkPipelineMultisampleStateCreateInfo MultisamplingStateCreateInfo();
	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();
	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

	VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
	VkImageViewCreateInfo ImageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

	VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
}
