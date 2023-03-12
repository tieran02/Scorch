#pragma once
#include "volk.h"
#include "render/pipeline.h"
#include "render/renderpass.h"

namespace vkutils 
{
	VkFormat ConvertFormat(SC::Format format);
	VkAttachmentLoadOp ConvertLoadOp(SC::AttachmentLoadOp loadOp);
	VkAttachmentStoreOp ConvertStoreOp(SC::AttachmentStoreOp storeOp);
	VkImageLayout ConvertImageLayout(SC::ImageLayout layout);
}
