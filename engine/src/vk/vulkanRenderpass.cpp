#include "pch.h"
#include "vk/vulkanRenderpass.h"
#include "vk/vulkanUtils.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"

using namespace SC;

namespace
{

}

VulkanRenderpass::VulkanRenderpass() : Renderpass(),
m_renderpass(VK_NULL_HANDLE)
{

}

bool VulkanRenderpass::Build()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;

	//Convert attachments to vk attachments
	std::vector<VkAttachmentDescription> vkAttachments(m_attachments.size());
	for (int i = 0; i < m_attachments.size(); ++i)
	{
		VkAttachmentDescription& attachment = vkAttachments[i];
		attachment.format = vkutils::ConvertFormat(m_attachments[i].format);
		attachment.samples = VK_SAMPLE_COUNT_1_BIT; //For multi sampling, not yet supported
		attachment.loadOp = vkutils::ConvertLoadOp(m_attachments[i].loadOp);
		attachment.storeOp = vkutils::ConvertStoreOp(m_attachments[i].storeOp);
		attachment.stencilLoadOp = vkutils::ConvertLoadOp(m_attachments[i].stencilLoadOp);
		attachment.stencilStoreOp = vkutils::ConvertStoreOp(m_attachments[i].stencilStoreOp);
		attachment.initialLayout = vkutils::ConvertImageLayout(m_attachments[i].initialLayout);
		attachment.finalLayout = vkutils::ConvertImageLayout(m_attachments[i].finalLayout);
	}

	//subpasses
	std::vector<VkAttachmentReference> colourReferences(m_colourReferences.size());
	for (int i = 0; i < m_colourReferences.size(); ++i)
	{
		colourReferences[i].attachment = m_colourReferences[i].index;
		colourReferences[i].layout = vkutils::ConvertImageLayout(m_colourReferences[i].layout);
	}

	VkAttachmentReference depthReference;
	if (m_depthReference.has_value()) 
	{
		depthReference.attachment = m_depthReference->index;
		depthReference.layout = vkutils::ConvertImageLayout(m_depthReference->layout);
	}


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colourReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colourReferences.size());
	subpass.pDepthStencilAttachment = m_depthReference.has_value() ? &depthReference : nullptr;

	//todo dependencies
	std::array<VkSubpassDependency, 2> dependencies;
	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	//build renderpass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = vkAttachments.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(vkAttachments.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());; //TODO
	renderPassInfo.pDependencies = dependencies.data(); //TODO

	VK_CHECK(vkCreateRenderPass(renderer->m_device, &renderPassInfo, nullptr, &m_renderpass));
	m_deletionQueue.push_function([=]()
		{
			vkDestroyRenderPass(renderer->m_device, m_renderpass, nullptr);
		});

	return m_renderpass != VK_NULL_HANDLE;
}

VulkanRenderpass::~VulkanRenderpass()
{
	m_deletionQueue.flush();
}

VkRenderPass VulkanRenderpass::GetRenderPass() const
{
	return m_renderpass;
}

