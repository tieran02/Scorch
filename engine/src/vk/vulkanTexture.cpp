#include "pch.h"
#include "vk/vulkanTexture.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanUtils.h"
#include "vk/vulkanInitialiser.h"

using namespace SC;

VulkanTexture::VulkanTexture(TextureType type, TextureUsage usage, Format format) : Texture(type, usage, format)
{

}

VulkanTexture::~VulkanTexture()
{
	m_deletionQueue.flush();
}

bool VulkanTexture::Build(uint32_t width, uint32_t height)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;

	VkExtent3D imageExtent = {
		width,
		height,
		1
	};

	m_width = width;
	m_height = height;

	VkFormat imageFormat = vkutils::ConvertFormat(m_format);
	VkImageUsageFlagBits usageFlags;
	switch (m_usage)
	{
	case SC::TextureUsage::DEPTH:
		usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	default:
		CORE_ASSERT(false, "Usage flag not supported");
		return false;
	}

	//the image will be an image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo img_info = vkinit::ImageCreateInfo(imageFormat, usageFlags, imageExtent);

	//we want to allocate it from GPU local memory
	VmaAllocationCreateInfo img_allocinfo = {};
	img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	img_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	VK_CHECK(vmaCreateImage(renderer->m_allocator, &img_info, &img_allocinfo, &m_image, &m_allocation, nullptr));

	//build an image-view for the image to use for rendering
	VkImageAspectFlagBits imageAspectFlags;
	switch (m_usage)
	{
	case SC::TextureUsage::DEPTH:
		imageAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	default:
		CORE_ASSERT(false, "Usage flag not supported");
		return false;
	}
	VkImageViewCreateInfo dview_info = vkinit::ImageviewCreateInfo(imageFormat, m_image, imageAspectFlags);

	VK_CHECK(vkCreateImageView(renderer->m_device, &dview_info, nullptr, &m_imageView));

	//add to deletion queues
	m_deletionQueue.push_function([=]() {
		vkDestroyImageView(renderer->m_device, m_imageView, nullptr);
		vmaDestroyImage(renderer->m_allocator, m_image, m_allocation);
		});

	return true;
}
