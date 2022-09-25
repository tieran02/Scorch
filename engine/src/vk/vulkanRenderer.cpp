#include "pch.h"
#include "vk/vulkanRenderer.h"
#include "VkBootstrap.h"
#include "volk.h"
#include "GLFW/glfw3.h"
#include "core/app.h"
#include "core/log.h"
#include "vk/vulkanInitialiser.h"
#include "core/shaderModule.h"
#include "vk/vulkanUtils.h"

using namespace SC;

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			CORE_ASSERT(false, string_format("%s %i", "Detected Vulkan error:", err)); \
			abort();                                                \
		}                                                           \
	} while (0)


VulkanRenderer::VulkanRenderer() : Renderer(),
	m_instance(VK_NULL_HANDLE)
{
	Init();
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

	InitPipelines();
}

void VulkanRenderer::Cleanup()
{
	Log::PrintCore("Cleaning up Vulkan Renderer");

	//Wait for rendering to finish before cleaning up
	VK_CHECK(vkWaitForFences(m_device, 1, &m_renderFence, true, 10000000));

	vkDestroyPipeline(m_device, m_trianglePipeline, nullptr);
	vkDestroyPipelineLayout(m_device, m_trianglePipelineLayout, nullptr);

	vkDestroyFence(m_device, m_renderFence, nullptr);
	vkDestroySemaphore(m_device, m_presentSemaphore, nullptr);
	vkDestroySemaphore(m_device, m_renderSemaphore, nullptr);

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	//destroy the main renderpass
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	//destroy swapchain resources
	for (int i = 0; i < m_swapchainImageViews.size(); i++)
	{
		vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}

	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
	vkDestroyInstance(m_instance, nullptr);
}

void VulkanRenderer::Draw()
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
	VK_CHECK(vkWaitForFences(m_device, 1, &m_renderFence, true, timeout));
	VK_CHECK(vkResetFences(m_device, 1, &m_renderFence));

	//request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(m_device, m_swapchain, timeout, m_presentSemaphore, nullptr, &swapchainImageIndex));

	//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(m_mainCommandBuffer, 0));

	//naming it cmd for shorter writing
	VkCommandBuffer cmd = m_mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


	//make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue;
	float flash = abs(sin(static_cast<float>(app->GetWindowTime()) / 120.f));
	clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rpInfo = vkinit::RenderpassBeginInfo(m_renderPass, VkExtent2D(windowWidth, windowHeight), m_framebuffers[swapchainImageIndex]);

	//connect clear values
	rpInfo.clearValueCount = 1;
	rpInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

	//do rendering here
	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_trianglePipeline);
	vkCmdDraw(cmd, 3, 1, 0, 0);

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
	submit.pWaitSemaphores = &m_presentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &m_renderSemaphore;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submit, m_renderFence));


	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = vkinit::PresentInfo();

	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores = &m_renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(m_graphicsQueue, &presentInfo));


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
	m_swapchainImages = vkbSwapchain.get_images().value();
	m_swapchainImageViews = vkbSwapchain.get_image_views().value();

	m_swapchainImageFormat = vkbSwapchain.image_format;
}

void VulkanRenderer::InitCommands()
{
	VkCommandPoolCreateInfo commandPoolInfo = vkinit::CommandPoolCreateInfo(m_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	VK_CHECK(vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool));

	//allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::CommandBufferAllocateInfo(m_commandPool, 1);
	VK_CHECK(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &m_mainCommandBuffer));
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

	//now create the renderpass
	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

	//connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment;
	//connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass;


	VK_CHECK(vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_renderPass));
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
	m_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

	//create framebuffers for each of the swapchain image views
	for (uint32_t i = 0; i < swapchain_imagecount; i++) {

		fb_info.pAttachments = &m_swapchainImageViews[i];
		VK_CHECK(vkCreateFramebuffer(m_device, &fb_info, nullptr, &m_framebuffers[i]));
	}
}

void VulkanRenderer::InitSyncStructures()
{
	//create synchronization structures

	VkFenceCreateInfo fenceCreateInfo = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECK(vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_renderFence));

	//for the semaphores we don't need any flags
	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_presentSemaphore));
	VK_CHECK(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_renderSemaphore));
}

void VulkanRenderer::InitPipelines()
{
	int windowWidth{ 0 }, windowHeight{ 0 };
	const App* app = App::Instance();
	if (!app)
	{
		Log::PrintCore("Failed to get app instance", LogSeverity::LogFatel);
		return;
	}
	app->GetWindowExtent(windowWidth, windowHeight);

	ShaderModuleBuilder shaderBuilder;
	auto shader = shaderBuilder.SetVertexModulePath("shaders/triangle.vert.spv")
		.SetFragmentModulePath("shaders/triangle.frag.spv")
		.Build();

	ShaderModuleArray<VkShaderModule> modules;
	LoadShaderModule(m_device, *shader, modules);

	//build the pipeline layout that controls the inputs/outputs of the shader
	//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
	VK_CHECK(vkCreatePipelineLayout(m_device, &pipeline_layout_info, nullptr, &m_trianglePipelineLayout));


	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	PipelineBuilder pipelineBuilder;
	VkShaderModule& vertexModule = modules.at(to_underlying(ShaderStage::VERTEX));
	if (vertexModule != VK_NULL_HANDLE)
	{
		pipelineBuilder._shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertexModule));
	}
	VkShaderModule& fragmentModule = modules.at(to_underlying(ShaderStage::FRAGMENT));
	if (fragmentModule != VK_NULL_HANDLE)
	{
		pipelineBuilder._shaderStages.push_back(vkinit::PipelineShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragmentModule));

	}

	//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
	pipelineBuilder._vertexInputInfo = vkinit::VertexInputStateCreateInfo();

	//input assembly is the configuration for drawing triangle lists, strips, or individual points.
	//we are just going to draw triangle list
	pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = 0.0f;
	pipelineBuilder._viewport.y = 0.0f;
	pipelineBuilder._viewport.width = (float)windowWidth;
	pipelineBuilder._viewport.height = (float)windowHeight;
	pipelineBuilder._viewport.minDepth = 0.0f;
	pipelineBuilder._viewport.maxDepth = 1.0f;

	pipelineBuilder._scissor.offset = { 0, 0 };
	pipelineBuilder._scissor.extent = VkExtent2D(windowWidth, windowHeight);

	//configure the rasterizer to draw filled triangles
	pipelineBuilder._rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);

	//we don't use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkinit::MultisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkinit::ColorBlendAttachmentState();

	//use the triangle layout we created
	pipelineBuilder._pipelineLayout = m_trianglePipelineLayout;

	//finally build the pipeline
	m_trianglePipeline = pipelineBuilder.BuildPipeline(m_device, m_renderPass);

	//finally destroy shader modules
	if (vertexModule)
		vkDestroyShaderModule(m_device, vertexModule, nullptr);
	if (fragmentModule)
		vkDestroyShaderModule(m_device, fragmentModule, nullptr);
}
