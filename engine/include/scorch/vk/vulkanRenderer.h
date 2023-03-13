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

	struct VulkanFrameData
	{
		VkSemaphore m_presentSemaphore, m_renderSemaphore;
		VkFence m_renderFence;

		VkCommandPool m_commandPool;
		VkCommandBuffer m_mainCommandBuffer;
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

		void BeginFrame(float clearR = 0, float clearG = 0, float clearB = 0) override;
		void EndFrame() override;

		void BeginRenderPass(const Renderpass* renderPass, float clearR = 0, float clearG = 0, float clearB = 0, float clearDepth = 0) override;
		void EndRenderPass() override;

		void SetViewport(const Viewport& viewport) override;
		void SetScissor(const Scissor& viewport) override;

		void BindPipeline(const Pipeline* pipeline) override;
		void BindVertexBuffer(const Buffer* buffer) override;
		void BindIndexBuffer(const Buffer* buffer) override;

		void BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet,int set = 0) override;
		void PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data) override;

		void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

		void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) const;

		void WaitOnFences() const;

		VulkanFrameData& GetCurrentFrame();
		const VulkanFrameData& GetCurrentFrame() const;

		VkRenderPass GetDefaultRenderPass() const;
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