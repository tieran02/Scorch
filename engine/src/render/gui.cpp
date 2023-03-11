#include "pch.h"
#include "render/gui.h"
#include "render/renderer.h"
#include "volk.h"
#include "vk/vulkanRenderer.h"

#include "backends/imgui_impl_vulkan.h"
#include "backends/imgui_impl_glfw.h"
#include "imgui.h"

using namespace SC;


#define ENABLE_VIEWPORTS

std::unique_ptr<GUI> GUI::Create(Renderer* renderer, GLFWwindow* window)
{
	CORE_ASSERT(renderer, "Renderer is null");
	if (!renderer) return nullptr;

	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		return std::unique_ptr<GUI>(new GUI(renderer, window));
	default:
		CORE_ASSERT(false, "GUI not supported for API");
		break;
	}

	return nullptr;
}

GUI::GUI(Renderer* renderer, GLFWwindow* window) : m_renderer(renderer), m_window(window)
{
	Init();
}

GUI::~GUI()
{
	Cleanup();
}

void GUI::Init()
{
	VulkanRenderer* vulanRenderer = static_cast<VulkanRenderer*>(m_renderer);
	//1: create descriptor pool for IMGUI
	// the size of the pool is very oversize, but it's copied from imgui demo itself.
	VkDescriptorPoolSize pool_sizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1000;
	pool_info.poolSizeCount = static_cast<uint32_t>(std::size(pool_sizes));
	pool_info.pPoolSizes = pool_sizes;

	VkDescriptorPool imguiPool;
	VkDevice device = vulanRenderer->m_device;
	VK_CHECK(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool));

	//this initializes the core structures of imgui
	ImGui::CreateContext();

	//docking
	ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;

    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

#ifdef ENABLE_VIEWPORTS
    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
#endif

	ImGui_ImplVulkan_LoadFunctions([](const char* function_name, void* vulkan_instance) {
		return vkGetInstanceProcAddr(*(reinterpret_cast<VkInstance*>(vulkan_instance)), function_name);
		}, &vulanRenderer->m_instance);

	//this initializes imgui for Glfw
	ImGui_ImplGlfw_InitForVulkan(m_window, true);

	//this initializes imgui for Vulkan
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vulanRenderer->m_instance;
	init_info.PhysicalDevice = vulanRenderer->m_chosenGPU;
	init_info.Device = vulanRenderer->m_device;
	init_info.Queue = vulanRenderer->m_graphicsQueue;
	init_info.DescriptorPool = imguiPool;
	init_info.MinImageCount = 3;
	init_info.ImageCount = 3;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.CheckVkResultFn = [](VkResult err)
		{
		if (err)
		{
			CORE_ASSERT(false, string_format("{0} {1}", "Detected Vulkan error:", err));
		}
		};

	ImGui_ImplVulkan_Init(&init_info, vulanRenderer->m_renderPass);

	//execute a gpu command to upload imgui font textures
	vulanRenderer->ImmediateSubmit([&](VkCommandBuffer cmd) 
		{
		ImGui_ImplVulkan_CreateFontsTexture(cmd);
		});

	//clear font textures from cpu data
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	m_deletionQueue.push_function([=]() {

		vkDestroyDescriptorPool(device, imguiPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		});
}

void GUI::Cleanup()
{
	m_deletionQueue.flush();
}

void GUI::BeginFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void GUI::EndFrame()
{
	ImGui::Render();

	VulkanRenderer* vulanRenderer = static_cast<VulkanRenderer*>(m_renderer);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), vulanRenderer->GetCurrentFrame().m_mainCommandBuffer);

#ifdef ENABLE_VIEWPORTS
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
#endif
}
