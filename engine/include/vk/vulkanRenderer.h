#pragma once
#include "core/renderer.h"
#include "volk.h"


class VulkanRenderer : public Renderer
{
public:
	VulkanRenderer();
	~VulkanRenderer();

	void Init() override;
	void Cleanup() override;
private:
	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debug_messenger; // Vulkan debug output handle
	VkPhysicalDevice m_chosenGPU; // GPU chosen as the default device
	VkDevice m_device; // Vulkan device for commands
	VkSurfaceKHR m_surface; // Vulkan window surface
};