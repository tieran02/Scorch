#include "pch.h"
#include "vk/vulkanUtils.h"
#include "render/pipeline.h"

using namespace SC;

VkFormat vkutils::ConvertFormat(Format format)
{
	switch (format)
	{
	case SC::Format::UNDEFINED:
		return VK_FORMAT_UNDEFINED;
	case SC::Format::D32_SFLOAT:
		return VK_FORMAT_D32_SFLOAT;
	case Format::R8_SRGB:
		return VK_FORMAT_R8_SRGB;
	case Format::R8G8_SRGB:
		return VK_FORMAT_R8G8_SRGB;
	case Format::R8G8B8_SRGB:
		return VK_FORMAT_R8G8B8_SRGB;
	case Format::R8G8B8A8_SRGB:
		return VK_FORMAT_R8G8B8A8_SRGB;
	case Format::B8G8R8A8_SRGB:
		return VK_FORMAT_B8G8R8A8_SRGB;
	case SC::Format::R32_SFLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case SC::Format::R32G32_SFLOAT:
		return VK_FORMAT_R32G32_SFLOAT;
	case SC::Format::R32G32B32_SFLOAT:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case SC::Format::R32G32B32A32_SFLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		CORE_ASSERT(false, "format not valid");
			return VK_FORMAT_UNDEFINED;
	}
}

VkAttachmentLoadOp vkutils::ConvertLoadOp(SC::AttachmentLoadOp loadOp)
{
	switch (loadOp)
	{
	case SC::AttachmentLoadOp::LOAD:
		return VK_ATTACHMENT_LOAD_OP_LOAD;
	case SC::AttachmentLoadOp::CLEAR:
		return VK_ATTACHMENT_LOAD_OP_CLEAR;
	case SC::AttachmentLoadOp::DONT_CARE:
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	default:
		CORE_ASSERT(false, "load op not valid");
		return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	}
}

VkAttachmentStoreOp vkutils::ConvertStoreOp(SC::AttachmentStoreOp storeOp)
{
	switch (storeOp)
	{
	case SC::AttachmentStoreOp::STORE:
		return VK_ATTACHMENT_STORE_OP_STORE;
	case SC::AttachmentStoreOp::DONT_CARE:
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	default:
		CORE_ASSERT(false, "store op not valid");
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
}

VkImageLayout vkutils::ConvertImageLayout(SC::ImageLayout layout)
{
	switch (layout)
	{
	case SC::ImageLayout::UNDEFINED:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case SC::ImageLayout::COLOR_ATTACHMENT_OPTIMAL:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	case SC::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case SC::ImageLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case SC::ImageLayout::DEPTH_ATTACHMENT_OPTIMAL:
		return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	case SC::ImageLayout::DEPTH_READ_ONLY_OPTIMAL:
		return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
	case SC::ImageLayout::SHADER_READ_ONLY_OPTIMAL:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case SC::ImageLayout::TRANSFER_SRC_OPTIMAL:
		return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	case SC::ImageLayout::TRANSFER_DST_OPTIMAL:
		return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	case SC::ImageLayout::PRESENT_SRC_KHR:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	default:
		CORE_ASSERT(false, "layout not valid");
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}
