#pragma once
#include "render/renderer.h"
#include "volk.h"
#include "core/utils.h"
#include "vk_mem_alloc.h"
#include "vulkanTexture.h"

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			CORE_ASSERT(false, string_format("{0} {1}", "Detected Vulkan error:", err)); \
			abort();                                                \
		}                                                           \
	} while (0)


namespace SC
{
	class VulkanRenderpass;
	class CommandPool;
	class CommandBuffer;

	struct VulkanFrameData
	{
		VkSemaphore m_presentSemaphore, m_renderSemaphore;
		VkFence m_renderFence;

		std::unique_ptr<CommandPool> m_commandPool;
		std::unique_ptr<CommandBuffer> m_mainCommandBuffer;
	};

	struct UploadContext
	{
		VkFence m_uploadFence;
		VkCommandPool m_commandPool;
		VkCommandBuffer m_commandBuffer;
	};

	class VulkanRenderer : public Renderer
	{
	public:
		VulkanRenderer();
		~VulkanRenderer();

		void Init() override;
		void Cleanup() override;
		void CreateSwapchain() override;

		void BeginFrame() override;
		void EndFrame() override;
		void SubmitCommandBuffer(const CommandBuffer& commandBuffer) override;

		CommandBuffer& GetFrameCommandBuffer() const override;

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) const;

		void WaitOnFences() const;

		VulkanFrameData& GetCurrentFrame();
		const VulkanFrameData& GetCurrentFrame() const;

		VkRenderPass GetDefaultRenderPass() const;

		Renderpass* DefaultRenderPass() const override;
		RenderTarget* DefaultRenderTarget() const override;

	private:
		void InitVulkan();
		void InitSwapchain();
		void InitCommands();

		void InitDefaultRenderpass();
		void InitFramebuffers();

		void InitSyncStructures();

		void InitDescriptors();
	public:
		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debug_messenger; // Vulkan debug output handle
		VkPhysicalDevice m_chosenGPU; // GPU chosen as the default device
		VkDevice m_device; // Vulkan device for commands
		VkSurfaceKHR m_surface; // Vulkan window surface

		VkSwapchainKHR m_swapchain; // from other articles
		VkFormat m_swapchainImageFormat; // image format expected by the windowing system
		std::vector<VulkanTexture> m_swapchainTextures;
		std::vector<std::unique_ptr<VulkanRenderTarget>> m_swapChainRenderTargets;


		VkQueue m_graphicsQueue; //queue we will submit to
		uint32_t m_graphicsQueueFamily; //family of that queue

		std::unique_ptr<VulkanRenderpass> m_vulkanRenderPass;

		VmaAllocator m_allocator; //vma lib allocator
		VkDescriptorPool m_descriptorPool;
	private:
		DeletionQueue m_mainDeletionQueue;
		DeletionQueue m_swapChainDeletionQueue;

		uint32_t m_swapchainImageIndex;

		UploadContext m_uploadContext;

		std::array<VulkanFrameData, FRAME_OVERLAP_COUNT[to_underlying(GraphicsAPI::VULKAN)]> m_frames;
	};
}