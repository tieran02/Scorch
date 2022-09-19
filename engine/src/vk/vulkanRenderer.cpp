#include "pch.h"
#include "vk/vulkanRenderer.h"
#include "VkBootstrap.h"
#include "volk.h"
#include "GLFW/glfw3.h"
#include "core/app.h"
#include "core/log.h"

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
}

void VulkanRenderer::Cleanup()
{
	Log::PrintCore("Cleaning up Vulkan Renderer");

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	//destroy swapchain resources
	for (int i = 0; i < m_swapchainImageViews.size(); i++) {

		vkDestroyImageView(m_device, m_swapchainImageViews[i], nullptr);
	}

	vkDestroyDevice(m_device, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkb::destroy_debug_utils_messenger(m_instance, m_debug_messenger);
	vkDestroyInstance(m_instance, nullptr);
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

}