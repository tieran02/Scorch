#include "pch.h"
#include "vk/vulkanRenderer.h"
#include "VkBootstrap.h"


VulkanRenderer::VulkanRenderer() : Renderer(),
	m_instance(VK_NULL_HANDLE)
{
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
}