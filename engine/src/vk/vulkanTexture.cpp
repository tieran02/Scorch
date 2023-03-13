#include "pch.h"
#include "vk/vulkanTexture.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanUtils.h"
#include "vk/vulkanInitialiser.h"
#include "vk/vulkanBuffer.h"
#include "jaam.h"

using namespace SC;

namespace
{
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) 
	{

		CORE_ASSERT(image != VK_NULL_HANDLE, "Image can't be null");
		if (image == VK_NULL_HANDLE) return;

		const App* app = App::Instance();
		CORE_ASSERT(app, "App instance is null");
		if (!app) return;

		const VulkanRenderer* renderer = app->GetVulkanRenderer();
		if (!renderer)
			return;

		renderer->ImmediateSubmit([&](VkCommandBuffer commandBuffer)
			{

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);


		});
	}
}

VulkanTexture::VulkanTexture(TextureType type, TextureUsage usage, Format format) : Texture(type, usage, format),
m_mipLevels(0)
{

}

VulkanTexture::~VulkanTexture()
{
	m_deletionQueue.flush();
}


bool VulkanTexture::Build(uint32_t width, uint32_t height, bool generateMipmaps)
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

	if (generateMipmaps)
		usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	m_mipLevels = generateMipmaps ?
		static_cast<uint32_t>(std::floor(std::log2(std::max(imageExtent.width, imageExtent.height)))) + 1 :
		1;

	//the image will be an image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo img_info = vkinit::ImageCreateInfo(imageFormat, usageFlags, imageExtent, m_mipLevels);

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
	VkImageViewCreateInfo dview_info = vkinit::ImageviewCreateInfo(imageFormat, m_image, imageAspectFlags, m_mipLevels);

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
	const uint32_t miplevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imageExtent.width, imageExtent.height)))) + 1;

	VkImageCreateInfo dimg_info = vkinit::ImageCreateInfo(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent, miplevels);
	VmaAllocationCreateInfo dimg_allocinfo = {};
	dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;


	//allocate and create the image
	vmaCreateImage(renderer->m_allocator, &dimg_info, &dimg_allocinfo, &m_image, &m_allocation, nullptr);

	CopyData(imageData.pixels.data(), imageData.Size());

	VkImageViewCreateInfo imageinfo = vkinit::ImageviewCreateInfo(image_format, m_image, VK_IMAGE_ASPECT_COLOR_BIT, miplevels);
	vkCreateImageView(renderer->m_device, &imageinfo, nullptr, &m_imageView);

	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkDestroyImageView(renderer->m_device, m_imageView, nullptr);
		vmaDestroyImage(renderer->m_allocator, m_image, m_allocation);
		});

	return m_image != VK_NULL_HANDLE;
}

bool VulkanTexture::CopyData(const void* data, size_t size)
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
		range.levelCount = m_mipLevels;
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

		//VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

		//imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		//imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		//imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		//imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		////barrier the image into the shader readable layout
		//vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
		});

	VkFormat imageFormat = vkutils::ConvertFormat(m_format);
	uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(m_width, m_height)))) + 1;
	generateMipmaps(m_image, imageFormat, m_width, m_height, mipLevels);
	return true;
}

VulkanRenderTarget::VulkanRenderTarget(std::vector<Format>&& attachmentFormats, uint32_t width, uint32_t height) : 
	RenderTarget(std::move(attachmentFormats), width, height)
{

}

bool VulkanRenderTarget::BuildAttachment(uint32_t attachmentIndex)
{
	CORE_ASSERT(attachmentIndex >= 0 && attachmentIndex < m_attachmentFormats.size(), "attachmentIndex is out of bounds");
	if(attachmentIndex < 0 && attachmentIndex >= m_attachmentFormats.size()) return false;

	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;

	VkExtent3D imageExtent = {
		m_width,
		m_height,
		1
	};


	VkFormat imageFormat = vkutils::ConvertFormat(m_attachmentFormats[attachmentIndex]);
	uint32_t usageFlags = m_attachmentFormats[attachmentIndex] == Format::D32_SFLOAT ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


	//the image will be an image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo img_info = vkinit::ImageCreateInfo(imageFormat, usageFlags, imageExtent, 1);

	//we want to allocate it from GPU local memory
	VmaAllocationCreateInfo img_allocinfo = {};
	img_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	img_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	//allocate and create the image
	VK_CHECK(vmaCreateImage(renderer->m_allocator, &img_info, &img_allocinfo, &m_image, &m_allocation, nullptr));

	//build an image-view for the image to use for rendering
	VkImageAspectFlagBits imageAspectFlags = m_attachmentFormats[attachmentIndex] == Format::D32_SFLOAT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo dview_info = vkinit::ImageviewCreateInfo(imageFormat, m_image, imageAspectFlags, 1);

	VK_CHECK(vkCreateImageView(renderer->m_device, &dview_info, nullptr, &m_imageView));

	//add to deletion queues
	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
	vkDestroyImageView(renderer->m_device, m_imageView, nullptr);
	vmaDestroyImage(renderer->m_allocator, m_image, m_allocation);
		});

	return true;
}

VulkanRenderTarget::~VulkanRenderTarget()
{
	m_deletionQueue.flush();
}

