#include "pch.h"

#include "vk/vulkanRenderer.h"
#include "VkBootstrap.h"
#include "volk.h"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

#include "GLFW/glfw3.h"
#include "core/app.h"
#include "core/log.h"
#include "vk/vulkanInitialiser.h"
#include "render/shaderModule.h"
#include "vk/vulkanUtils.h"
#include "vk/vulkanPipeline.h"
#include "vk/vulkanBuffer.h"
#include "vk/vulkanTexture.h"
#include "render/mesh.h"
#include "vk/vulkanDescriptorSet.h"
#include "vk/vulkanRenderpass.h"
#include "render/commandbuffer.h"
#include "vk/vulkanCommandbuffer.h"

using namespace SC;

VulkanRenderer::VulkanRenderer() : Renderer(GraphicsAPI::VULKAN),
	m_instance(VK_NULL_HANDLE),
	m_descriptorPool(VK_NULL_HANDLE)
{
}

VulkanRenderer::~VulkanRenderer()
{
	Cleanup();
}

void VulkanRenderer::Init()
{
	Log::PrintCore("Creating Vulkan Renderer");

	InitVulkan();
	InitSwapchain();
	InitCommands();

	InitSyncStructures();

	InitDefaultRenderpass();
	InitFramebuffers();

	InitDescriptors();

	Renderer::Init();
}

void VulkanRenderer::Cleanup()
{
	Renderer::Cleanup();

	Log::PrintCore("Cleaning up Vulkan Renderer");

	//Wait for rendering to finish before cleaning up
	VK_CHECK(vkWaitForFences(m_device, 1, &GetCurrentFrame().m_renderFence, true, 10000000));

	m_swapChainDeletionQueue.flush();
	m_mainDeletionQueue.flush();

	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
	vkDestroyInstance(m_instance, nullptr);
}

void VulkanRenderer::CreateSwapchain()
{
	Log::PrintCore("Creating swapchain");

	//Wait for rendering to finish before cleaning up
	VK_CHECK(vkWaitForFences(m_device, 1, &GetCurrentFrame().m_renderFence, true, 10000000));

	m_swapChainDeletionQueue.flush();

	InitSwapchain();
	InitFramebuffers();
}

void VulkanRenderer::BeginFrame()
{
	int windowWidth{ 0 }, windowHeight{ 0 };
	const App* app = App::Instance();
	if (!app)
	{
		Log::PrintCore("Failed to get app instance", LogSeverity::LogFatel);
		return;
	}
	app->GetWindowExtent(windowWidth, windowHeight);

	constexpr uint32_t timeout = 1000000000;
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(m_device, 1, &GetCurrentFrame().m_renderFence, true, timeout));
	VK_CHECK(vkResetFences(m_device, 1, &GetCurrentFrame().m_renderFence));

	//request image from the swapchain, one second timeout
	VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, timeout, GetCurrentFrame().m_presentSemaphore, nullptr, &m_swapchainImageIndex));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	GetFrameCommandBuffer().ResetCommands();
	GetFrameCommandBuffer().BeginRecording();
}


void VulkanRenderer::EndFrame()
{
	//naming it cmd for shorter writing
	VulkanCommandBuffer& cmd = static_cast<VulkanCommandBuffer&>(GetFrameCommandBuffer());

	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	cmd.EndRecording();

	//prepare the submission to the queue.
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished
	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd.GetCommandBuffer());
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &GetCurrentFrame().m_presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &GetCurrentFrame().m_renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, GetCurrentFrame().m_renderFence));


	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::PresentInfo();

	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &GetCurrentFrame().m_renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &m_swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(m_graphicsQueue, &presentInfo));

	m_currentFrame = m_currentFrame + 1 % std::numeric_limits<uint32_t>().max();
}

void VulkanRenderer::BeginRenderPass(const Renderpass* renderPass, const RenderTarget* renderTarget, float clearR /*= 0*/, float clearG /*= 0*/, float clearB /*= 0*/, float clearDepth /*= 0*/)
{
	const VulkanRenderpass* vulkanRenderpass = renderPass ? static_cast<const VulkanRenderpass*>(renderPass) : m_vulkanRenderPass.get();
	const VulkanRenderTarget* vulkanRenderTarget = renderTarget ? static_cast<const VulkanRenderTarget*>(renderTarget) : m_swapChainRenderTargets[m_swapchainImageIndex].get();

	GetFrameCommandBuffer().BeginRenderPass(vulkanRenderpass, vulkanRenderTarget, clearR, clearG, clearB, clearDepth);
}

void VulkanRenderer::EndRenderPass()
{
	GetFrameCommandBuffer().EndRenderPass();
}


void VulkanRenderer::SetViewport(const Viewport& viewport)
{
	GetFrameCommandBuffer().SetViewport(viewport);
}

void VulkanRenderer::SetScissor(const Scissor& scissor)
{
	GetFrameCommandBuffer().SetScissor(scissor);
}

void VulkanRenderer::BindPipeline(const Pipeline* pipeline)
{
	GetFrameCommandBuffer().BindPipeline(pipeline);
}


void VulkanRenderer::BindVertexBuffer(const Buffer* buffer)
{
	GetFrameCommandBuffer().BindVertexBuffer(buffer);
}

void VulkanRenderer::BindIndexBuffer(const Buffer* buffer)
{
	GetFrameCommandBuffer().BindIndexBuffer(buffer);
}

void VulkanRenderer::BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet, int set)
{
	GetFrameCommandBuffer().BindDescriptorSet(pipelineLayout, descriptorSet, set);
}

void VulkanRenderer::PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data)
{
	GetFrameCommandBuffer().PushConstants(pipelineLayout, rangeIndex, offset, size, data);
}

void VulkanRenderer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	GetFrameCommandBuffer().Draw(vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	GetFrameCommandBuffer().DrawIndexed(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRenderer::InitVulkan()
{
	VK_CHECK(volkInitialize());

	vkb::InstanceBuilder builder;

	//make the Vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
#ifdef SCORCH_DEBUG
		.request_validation_layers(true)
#else
		.request_validation_layers(false)
#endif
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//store the instance
	m_instance = vkb_inst.instance;

	volkLoadInstance(m_instance);

	m_debug_messenger = vkb_inst.debug_messenger;

	VK_CHECK(glfwCreateWindowSurface(m_instance, App::Instance()->GetWindowHandle(), VK_NULL_HANDLE, &m_surface));

	// use vkbootstrap to select a GPU.
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(m_surface)
		.require_present()
		.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
		.allow_any_gpu_device_type(false)
		.select()
		.value();

	Log::PrintCore(string_format("Device: {0}", physicalDevice.name));

	//create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };
	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	m_device = vkbDevice.device;
	m_chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	m_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	m_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

	//initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_chosenGPU;
	allocatorInfo.device = m_device;
	allocatorInfo.instance = m_instance;
	vmaCreateAllocator(&allocatorInfo, &m_allocator);

	m_mainDeletionQueue.push_function([=]() {
		vmaDestroyAllocator(m_allocator);
		});
}

void VulkanRenderer::InitSwapchain()
{
	vkb::SwapchainBuilder swapchainBuilder{ m_chosenGPU,m_device,m_surface };

	int windowWidth{0}, windowHeight{0};
	const App* app = App::Instance();
	if (!app)
	{
		Log::PrintCore("Failed to get app instance", LogSeverity::LogFatel);
		return;
	}
	app->GetWindowExtent(windowWidth, windowHeight);

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(windowWidth, windowHeight)
		.build()
		.value();

	//store swapchain and its related images
	m_swapchain = vkbSwapchain.swapchain;
	auto swapchainImages = vkbSwapchain.get_images().value();
	auto swapchainImageViews = vkbSwapchain.get_image_views().value();

	m_swapchainImageFormat = vkbSwapchain.image_format;

	m_swapChainRenderTargets.resize(swapchainImages.size());
	m_swapchainTextures.reserve(swapchainImages.size());

	for (int i = 0; i < m_swapChainRenderTargets.size(); ++i)
	{
		//Hold swapchain texture data inside VulkanTexture
		//As the swapchain has ownership there is no need for VulkanTexture to cleanup these resources (Just wrap them inside VulkanTexture)
		m_swapchainTextures.emplace_back(TextureType::TEXTURE2D, TextureUsage::COLOUR, Format::B8G8R8A8_SRGB);
		m_swapchainTextures[i].m_image = swapchainImages[i];
		m_swapchainTextures[i].m_imageView = swapchainImageViews[i];

		m_swapChainRenderTargets[i] = std::make_unique<VulkanRenderTarget>(std::vector<Format>{Format::B8G8R8A8_SRGB, Format::D32_SFLOAT}, windowWidth, windowHeight);
		m_swapChainRenderTargets[i]->SetAttachmentTexture(0, &m_swapchainTextures[i]);
		m_swapChainRenderTargets[i]->BuildAttachmentTexture(1);

		m_swapChainDeletionQueue.push_function([=]() {
			vkDestroyImageView(m_device, swapchainImageViews[i], nullptr);
			m_swapChainRenderTargets[i].reset();
			});
	}



	m_swapChainDeletionQueue.push_function([=]() {
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	});
}

void VulkanRenderer::InitCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < m_frames.size(); i++) {
		m_frames[i].m_commandPool = CommandPool::Create();
		m_frames[i].m_mainCommandBuffer = m_frames[i].m_commandPool->CreateCommandBuffer();
	}

	VkCommandPoolCreateInfo uploadCommandPoolInfo = vkinit::CommandPoolCreateInfo(m_graphicsQueueFamily);
	//create pool for upload context
	VK_CHECK(vkCreateCommandPool(m_device, &uploadCommandPoolInfo, nullptr, &m_uploadContext.m_commandPool));

	m_mainDeletionQueue.push_function([=]() {
		vkDestroyCommandPool(m_device, m_uploadContext.m_commandPool, nullptr);
		});

	//allocate the default command buffer that we will use for the instant commands
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_uploadContext.m_commandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_uploadContext.m_commandBuffer));
}

void VulkanRenderer::InitDefaultRenderpass()
{
	m_vulkanRenderPass = std::make_unique<VulkanRenderpass>();


	RenderpassAttachment colourAttachment;
	colourAttachment.format = Format::B8G8R8A8_SRGB;
	colourAttachment.loadOp = AttachmentLoadOp::CLEAR; // we Clear when this attachment is loaded
	colourAttachment.storeOp = AttachmentStoreOp::STORE;// we keep the attachment stored when the renderpass ends
	colourAttachment.stencilLoadOp = AttachmentLoadOp::DONT_CARE; 	//we don't care about stencil
	colourAttachment.stencilStoreOp = AttachmentStoreOp::DONT_CARE;
	colourAttachment.initialLayout = ImageLayout::UNDEFINED;
	colourAttachment.finalLayout = ImageLayout::PRESENT_SRC_KHR;
	m_vulkanRenderPass->AddAttachment(std::move(colourAttachment));

	m_vulkanRenderPass->AddColourReference(0, ImageLayout::COLOR_ATTACHMENT_OPTIMAL);


	RenderpassAttachment depthAttachment;
	depthAttachment.format = Format::D32_SFLOAT;
	depthAttachment.loadOp = AttachmentLoadOp::CLEAR; // we Clear when this attachment is loaded
	depthAttachment.storeOp = AttachmentStoreOp::STORE;// we keep the attachment stored when the renderpass ends
	depthAttachment.stencilLoadOp = AttachmentLoadOp::CLEAR; 	//we don't care about stencil
	depthAttachment.stencilStoreOp = AttachmentStoreOp::STORE;
	depthAttachment.initialLayout = ImageLayout::UNDEFINED;
	depthAttachment.finalLayout = ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	m_vulkanRenderPass->AddAttachment(std::move(depthAttachment));

	m_vulkanRenderPass->AddDepthReference(1, ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	m_vulkanRenderPass->Build();


	m_mainDeletionQueue.push_function([=]() 
		{
			m_vulkanRenderPass.reset();
		});
}

void VulkanRenderer::InitFramebuffers()
{
	for (int i = 0; i < m_swapChainRenderTargets.size(); ++i)
	{
		m_swapChainRenderTargets[i]->Build(m_vulkanRenderPass.get());
	}
}

void VulkanRenderer::InitSyncStructures()
{
	//create synchronization structures

	VkFenceCreateInfo fenceCreateInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	for (int i = 0; i < m_frames.size(); i++)
	{
		VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_frames[i].m_renderFence));

		m_mainDeletionQueue.push_function([=]() {
			vkDestroyFence(m_device, m_frames[i].m_renderFence, nullptr);
			});

		//for the semaphores we don't need any flags

		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i].m_presentSemaphore));
		VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_frames[i].m_renderSemaphore));

		m_mainDeletionQueue.push_function([=]() {

			vkDestroySemaphore(m_device, m_frames[i].m_presentSemaphore, nullptr);
			vkDestroySemaphore(m_device, m_frames[i].m_renderSemaphore, nullptr);
			});
	}

	VkFenceCreateInfo uploadFenceCreateInfo = vkinit::FenceCreateInfo();
	VK_CHECK(vkCreateFence(m_device, &uploadFenceCreateInfo, nullptr, &m_uploadContext.m_uploadFence));
	m_mainDeletionQueue.push_function([=]() {
		vkDestroyFence(m_device, m_uploadContext.m_uploadFence, nullptr);
		});
}

VulkanFrameData& VulkanRenderer::GetCurrentFrame()
{
	return m_frames.at(m_currentFrame % m_frames.size());
}

const SC::VulkanFrameData& VulkanRenderer::GetCurrentFrame() const
{
	return m_frames.at(m_currentFrame % m_frames.size());
}

void VulkanRenderer::WaitOnFences() const
{
	for (int i = 0; i < m_frames.size(); ++i)
	{
		vkWaitForFences(m_device, 1, &m_frames[i].m_renderFence, true, 10000000);
	}
}

void VulkanRenderer::InitDescriptors()
{
	//create a descriptor pool that will hold 10 uniform buffers
	std::vector<VkDescriptorPoolSize> sizes =
	{
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = (uint32_t)sizes.size();
	pool_info.pPoolSizes = sizes.data();

	vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptorPool);

	// add descriptor set layout to deletion queues
	m_mainDeletionQueue.push_function([&]() {
		vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
		});
}

void VulkanRenderer::ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) const
{
	VkCommandBuffer cmd = m_uploadContext.m_commandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//execute the function
	function(cmd);

	VK_CHECK(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_uploadContext.m_uploadFence));

	vkWaitForFences(m_device, 1, &m_uploadContext.m_uploadFence, true, 9999999999);
	vkResetFences(m_device, 1, &m_uploadContext.m_uploadFence);

	// reset the command buffers inside the command pool
	vkResetCommandPool(m_device, m_uploadContext.m_commandPool, 0);
}

VkRenderPass VulkanRenderer::GetDefaultRenderPass() const
{
	CORE_ASSERT(m_vulkanRenderPass, "Render pass not created");
	return m_vulkanRenderPass->GetRenderPass();
}

SC::CommandBuffer& VulkanRenderer::GetFrameCommandBuffer()
{
	return *m_frames[m_currentFrame % m_frames.size()].m_mainCommandBuffer;
}
