#pragma once
#include "core/renderer.h"
#include "volk.h"

namespace SC
{
	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer() override;

		void Init() override;
		void Cleanup() override;

		void Draw() override;
	private:
		void InitVulkan();
		void InitSwapchain();
		void InitCommands();

		void InitDefaultRenderpass();
		void InitFramebuffers();

		void InitSyncStructures();
	private:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debug_messenger; // Vulkan debug output handle
		VkPhysicalDevice m_chosenGPU; // GPU chosen as the default device
		VkDevice m_device; // Vulkan device for commands
		VkSurfaceKHR m_surface; // Vulkan window surface

		VkSwapchainKHR m_swapchain; // from other articles
		VkFormat m_swapchainImageFormat; // image format expected by the windowing system
		std::vector<VkImage> m_swapchainImages; //array of images from the swapchain
		std::vector<VkImageView> m_swapchainImageViews; //array of image-views from the swapchain

		VkQueue m_graphicsQueue; //queue we will submit to
		uint32_t m_graphicsQueueFamily; //family of that queue

		VkCommandPool m_commandPool; //the command pool for our commands
		VkCommandBuffer m_mainCommandBuffer; //the buffer we will record into

		VkRenderPass m_renderPass;
		std::vector<VkFramebuffer> m_framebuffers;

		VkSemaphore m_presentSemaphore, m_renderSemaphore;
		VkFence m_renderFence;
	};
}