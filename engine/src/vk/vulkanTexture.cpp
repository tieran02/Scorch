#include "pch.h"
#include "vk/vulkanTexture.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanUtils.h"
#include "vk/vulkanInitialiser.h"
#include "vk/vulkanBuffer.h"

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
	uint32_t usageFlags;
	switch (m_usage)
	{
	case SC::TextureUsage::DEPTH:
		usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	case SC::TextureUsage::COLOUR:
		usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
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
	case SC::TextureUsage::COLOUR:
		imageAspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	default:
		CORE_ASSERT(false, "Usage flag not supported");
		return false;
	}
	VkImageViewCreateInfo dview_info = vkinit::ImageviewCreateInfo(imageFormat, m_image, imageAspectFlags);

	VK_CHECK(vkCreateImageView(renderer->m_device, &dview_info, nullptr, &m_imageView));

	//add to deletion queues
	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkDestroyImageView(renderer->m_device, m_imageView, nullptr);
		vmaDestroyImage(renderer->m_allocator, m_image, m_allocation);
		});

	return true;
}

bool VulkanTexture::LoadFromFile(const std::string& path)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;

	ImageData imageData;
	if (!ReadImageFromFile(path, imageData))
		return false;

	m_width = static_cast<uint32_t>(imageData.width);
	m_height = static_cast<uint32_t>(imageData.height);

	VkFormat image_format = vkutils::ConvertFormat(m_format);
	VkExtent3D imageExtent{m_width,m_height,1};

	VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	//allocate and create the image
	vmaCreateImage(renderer->m_allocator, &dimg_info, &dimg_allocinfo, &m_image, &m_allocation, nullptr);

	CopyData(imageData.pixels.data(), imageData.Size());

	VkImageViewCreateInfo imageinfo = vkinit::ImageviewCreateInfo(image_format, m_image, VK_IMAGE_ASPECT_COLOR_BIT);
	vkCreateImageView(renderer->m_device, &imageinfo, nullptr, &m_imageView);

	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkDestroyImageView(renderer->m_device, m_imageView, nullptr);
		vmaDestroyImage(renderer->m_allocator, m_image, m_allocation);
		});

	return m_image != VK_NULL_HANDLE;
}

bool VulkanTexture::CopyData(void* data, size_t size)
{
	//TODO check if texture has dst flag

	CORE_ASSERT(m_image != VK_NULL_HANDLE, "Image can't be null");
	if (m_image == VK_NULL_HANDLE) return false;

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;

	BufferUsageSet stagingBufferUsage;
	stagingBufferUsage.set(BufferUsage::TRANSFER_SRC);
	stagingBufferUsage.set(BufferUsage::MAP);
	VulkanBuffer stagingBuffer(size, stagingBufferUsage, AllocationUsage::HOST, data);

	//now copy from the staging buffer into the texture
	renderer->ImmediateSubmit([&](VkCommandBuffer cmd) {
		VkImageSubresourceRange range;
		range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.baseMipLevel = 0;
		range.levelCount = 1;
		range.baseArrayLayer = 0;
		range.layerCount = 1;

		VkImageMemoryBarrier imageBarrier_toTransfer = {};
		imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toTransfer.image = m_image;
		imageBarrier_toTransfer.subresourceRange = range;

		imageBarrier_toTransfer.srcAccessMask = 0;
		imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

		VkBufferImageCopy copyRegion = {};
		copyRegion.bufferOffset = 0;
		copyRegion.bufferRowLength = 0;
		copyRegion.bufferImageHeight = 0;

		copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copyRegion.imageSubresource.mipLevel = 0;
		copyRegion.imageSubresource.baseArrayLayer = 0;
		copyRegion.imageSubresource.layerCount = 1;
		copyRegion.imageExtent = { m_width , m_height, 1};

		//copy the buffer into the image
		vkCmdCopyBufferToImage(cmd, *stagingBuffer.GetBuffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

		VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

		imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		//barrier the image into the shader readable layout
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
		});

	return true;
}
