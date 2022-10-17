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
	case SC::Format::R32_SFLOAT:
		return VK_FORMAT_R32_SFLOAT;
	case SC::Format::R32G32_SFLOAT:
		return VK_FORMAT_R32G32_SFLOAT;
	case SC::Format::R32G32B32_SFLOAT:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case SC::Format::R32G32B32A32_SFLOAT:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:
		CORE_ASSERT(false, "format not valid")
			return VK_FORMAT_UNDEFINED;
	}
}
