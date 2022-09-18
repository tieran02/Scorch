#pragma once
#include "core/renderer.h"
#include "vulkan/vulkan.h"


class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer();
private:
	VkInstance m_instance;
};