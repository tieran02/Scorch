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
	InitDefaultRenderpass();
	InitFramebuffers();

	InitSyncStructures();

	InitDescriptors();
}

void VulkanRenderer::Cleanup()
{
	Log::PrintCore("Cleaning up Vulkan Renderer");

	//Wait for rendering to finish before cleaning up
	VK_CHECK(vkWaitForFences(m_device, 1, &GetCurrentFrame().m_renderFence, true, 10000000));

	m_depthTexture.reset();
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

	m_depthTexture.reset();
	m_swapChainDeletionQueue.flush();

	InitSwapchain();
	InitFramebuffers();
}

void VulkanRenderer::BeginFrame(float r, float g, float b)
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
	VK_CHECK(vkResetCommandBuffer(GetCurrentFrame().m_mainCommandBuffer, 0));

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue;
	clearValue.color = { { r, g, b, 1.0f } };

	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(m_renderPass, VkExtent2D(windowWidth, windowHeight), m_swapChainFramebuffers[m_swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 2;
	VkClearValue clearValues[] = { clearValue, depthClear };
	rpInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void VulkanRenderer::EndFrame()
{
	//naming it cmd for shorter writing
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	//finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	//prepare the submission to the queue.
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished
	VkSubmitInfo submit = vkinit::SubmitInfo(&cmd);
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

void VulkanRenderer::SetViewport(const Viewport& viewport)
{
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	//VkViewport and viewport use the same memory layout so just reinterpret_cast
	const VkViewport& vkViewport = reinterpret_cast<const VkViewport&>(viewport);
	vkCmdSetViewport(cmd, 0, 1, &vkViewport);
}

void VulkanRenderer::SetScissor(const Scissor& scissor)
{
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	//VkRect2D and Scissor use the same memory layout so just reinterpret_cast
	const VkRect2D& vkScissor = reinterpret_cast<const VkRect2D&>(scissor);
	vkCmdSetScissor(cmd, 0, 1, &vkScissor);
}

void VulkanRenderer::BindPipeline(const Pipeline* pipeline)
{
	CORE_ASSERT(pipeline, "Pipline can't be null");
	//naming it cmd for shorter writing
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, static_cast<const VulkanPipeline*>(pipeline)->GetPipeline());
}


void VulkanRenderer::BindVertexBuffer(const Buffer* buffer)
{
	CORE_ASSERT(buffer, "Buffer can't be null");
	if (!buffer->HasUsage(BufferUsage::VERTEX_BUFFER))
	{
		CORE_ASSERT(false, "Buffer must be a vertex buffer");
		return;
	}
	
	//naming it cmd for shorter writing
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmd, 0, 1, static_cast<const VulkanBuffer*>(buffer)->GetBuffer(), &offset);
}

void VulkanRenderer::BindIndexBuffer(const Buffer* buffer)
{
	CORE_ASSERT(buffer, "Buffer can't be null");
	if (!buffer->HasUsage(BufferUsage::INDEX_BUFFER))
	{
		CORE_ASSERT(false, "Buffer must be a index buffer");
		return;
	}

	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	VkDeviceSize offset = 0;
	vkCmdBindIndexBuffer(cmd, *static_cast<const VulkanBuffer*>(buffer)->GetBuffer(), offset, sizeof(VertexIndexType) == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);
}

void VulkanRenderer::BindDescriptorSet(const PipelineLayout* pipelineLayout, const DescriptorSet* descriptorSet, int set)
{
	CORE_ASSERT(descriptorSet, "descriptorSet can't be null");

	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	const VulkanDescriptorSet* vulkanDescriptorSet = static_cast<const VulkanDescriptorSet*>(descriptorSet);
	VkPipelineLayout layout = static_cast<const VulkanPipelineLayout*>(pipelineLayout)->GetPipelineLayout();

	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, set, 1, &vulkanDescriptorSet->m_descriptorSet, 0, nullptr);
}

void VulkanRenderer::PushConstants(const PipelineLayout* pipelineLayout, uint32_t rangeIndex, uint32_t offset, uint32_t size, void* data)
{
	CORE_ASSERT(pipelineLayout, "Pipline layout can't be null");
	//naming it cmd for shorter writing
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;

	VkPipelineLayout layout = static_cast<const VulkanPipelineLayout*>(pipelineLayout)->GetPipelineLayout();
	if (rangeIndex < 0 && rangeIndex >= pipelineLayout->PushConstants().size())
	{
		CORE_ASSERT(false, "Range index out of range");
		return;
	}

	uint32_t shaderStages = 0;
	if (pipelineLayout->PushConstants()[rangeIndex].shaderStages.test(ShaderStage::VERTEX))
		shaderStages |= VK_SHADER_STAGE_VERTEX_BIT;
	if (pipelineLayout->PushConstants()[rangeIndex].shaderStages.test(ShaderStage::FRAGMENT))
		shaderStages |= VK_SHADER_STAGE_FRAGMENT_BIT;

	vkCmdPushConstants(cmd, layout, shaderStages, offset, pipelineLayout->PushConstants()[rangeIndex].size, data);
}

void VulkanRenderer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;
	vkCmdDraw(cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	VkCommandBuffer cmd = GetCurrentFrame().m_mainCommandBuffer;
	vkCmdDrawIndexed(cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanRenderer::InitVulkan()
{
	VK_CHECK(volkInitialize());

	vkb::InstanceBuilder builder;

	//make the Vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(true)
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
		.select()
		.value();

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
		.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR)
		.set_desired_extent(windowWidth, windowHeight)
		.build()
		.value();

	//store swapchain and its related images
	m_swapchain = vkbSwapchain.swapchain;
	m_swapchainImages = vkbSwapchain.get_images().value();
	m_swapchainImageViews = vkbSwapchain.get_image_views().value();

	m_swapchainImageFormat = vkbSwapchain.image_format;

	m_swapChainDeletionQueue.push_function([=]() {
		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	});


	//create depth image
	m_depthTexture = std::make_unique<VulkanTexture>(TextureType::TEXTURE2D, TextureUsage::DEPTH, Format::D32_SFLOAT);
	m_depthTexture->Build(windowWidth, windowHeight);
}

void VulkanRenderer::InitCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	for (int i = 0; i < m_frames.size(); i++) {
		VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_frames[i].m_commandPool));

		//allocate the default command buffer that we will use for rendering
		VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_frames[i].m_commandPool, 1);
		VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_frames[i].m_mainCommandBuffer));

		m_mainDeletionQueue.push_function([=]() {
			vkDestroyCommandPool(m_device, m_frames[i].m_commandPool, nullptr);
			});
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
	// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = m_swapchainImageFormat;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we don't care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depth_attachment = {};
	// Depth attachment
	depth_attachment.flags = 0;
	depth_attachment.format = VK_FORMAT_D32_SFLOAT;
	depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_attachment_ref = {};
	depth_attachment_ref.attachment = 1;
	depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//subpass
	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	//hook the depth attachment into the subpass
	subpass.pDepthStencilAttachment = &depth_attachment_ref;

	//now create the renderpass
	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//array of 2 attachments, one for the color, and other for depth
	std::vector<VkAttachmentDescription> attachments = { color_attachment,depth_attachment };
	render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
	render_pass_info.pAttachments = attachments.data();
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkSubpassDependency depth_dependency = {};
	depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	depth_dependency.dstSubpass = 0;
	depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.srcAccessMask = 0;
	depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::vector<VkSubpassDependency> dependencies = { dependency,depth_dependency };
	render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
	render_pass_info.pDependencies = dependencies.data();


	VK_CHECK(vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_renderPass));

	m_mainDeletionQueue.push_function([=]() {
		vkDestroyRenderPass(m_device, m_renderPass, nullptr);
		});
}

void VulkanRenderer::InitFramebuffers()
{
	int windowWidth{ 0 }, windowHeight{ 0 };
	const App* app = App::Instance();
	if (!app)
	{
		Log::PrintCore("Failed to get app instance", LogSeverity::LogFatel);
		return;
	}
	app->GetWindowExtent(windowWidth, windowHeight);

	//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = vkinit::FramebufferCreateInfo(m_renderPass, VkExtent2D(windowWidth, windowHeight));

	//grab how many images we have in the swapchain
	const uint32_t swapchain_imagecount = static_cast<uint32_t>(m_swapchainImages.size());
	m_swapChainFramebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (uint32_t i = 0; i < swapchain_imagecount; i++) 
	{
		VkImageView attachments[2];
		attachments[0] = m_swapchainImageViews[i];
		attachments[1] = m_depthTexture->m_imageView;


		fb_info.pAttachments = attachments;
		fb_info.attachmentCount = 2;

		VK_CHECK(vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_swapChainFramebuffers[i]));

		m_swapChainDeletionQueue.push_function([=]() {
			vkDestroyFramebuffer(m_device, m_swapChainFramebuffers[i], nullptr);
			vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
			});
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
	pool_info.flags = 0;
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

