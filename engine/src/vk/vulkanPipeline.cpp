#include "pch.h"
#include "vk/vulkanPipeline.h"
#include "core/shaderModule.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanInitialiser.h"

using namespace SC;

namespace
{
	bool LoadShaderModuleVk(VkDevice device, const ShaderBufferType& buffer, VkShaderModule& outShaderModule)
	{
		//create a new shader module, using the buffer we loaded
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.pNext = nullptr;

		//codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
		createInfo.codeSize = buffer.size() * sizeof(uint32_t);
		createInfo.pCode = buffer.data();

		//check that the creation goes well.
		if (vkCreateShaderModule(device, &createInfo, nullptr, &outShaderModule) != VK_SUCCESS)
			return false;

		return true;
	}

	bool LoadShaderModule(VkDevice device, const ShaderModule& module, ShaderModuleArray<VkShaderModule>& outShaderModules)
	{
		bool success = true;
		for (uint8_t i = 0; i < to_underlying(ShaderStage::COUNT); ++i)
		{
			const ShaderBufferType buffer = module.GetModule(static_cast<ShaderStage>(i));
			if (buffer.empty())
				continue;

			success = LoadShaderModuleVk(device, buffer, outShaderModules[i]);
		}

		return success;
	}

	class VkPipelineBuilder
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
		VkViewport _viewport;
		VkRect2D _scissor;
		VkPipelineRasterizationStateCreateInfo _rasterizer;
		VkPipelineColorBlendAttachmentState _colorBlendAttachment;
		VkPipelineMultisampleStateCreateInfo _multisampling;
		VkPipelineLayout _pipelineLayout;

		VkPipeline BuildPipeline(VkDevice device, VkRenderPass pass)
		{
			//make viewport state from our stored viewport and scissor.
			//at the moment we won't support multiple viewports or scissors
			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = nullptr;

			viewportState.viewportCount = 1;
			viewportState.pViewports = &_viewport;
			viewportState.scissorCount = 1;
			viewportState.pScissors = &_scissor;

			//setup dummy color blending. We aren't using transparent objects yet
			//the blending is just "no blend", but we do write to the color attachment
			VkPipelineColorBlendStateCreateInfo colorBlending = {};
			colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlending.pNext = nullptr;

			colorBlending.logicOpEnable = VK_FALSE;
			colorBlending.logicOp = VK_LOGIC_OP_COPY;
			colorBlending.attachmentCount = 1;
			colorBlending.pAttachments = &_colorBlendAttachment;

			//build the actual pipeline
			//we now use all of the info structs we have been writing into into this one to create the pipeline
			VkGraphicsPipelineCreateInfo pipelineInfo = {};
			pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineInfo.pNext = nullptr;

			pipelineInfo.stageCount = _shaderStages.size();
			pipelineInfo.pStages = _shaderStages.data();
			pipelineInfo.pVertexInputState = &_vertexInputInfo;
			pipelineInfo.pInputAssemblyState = &_inputAssembly;
			pipelineInfo.pViewportState = &viewportState;
			pipelineInfo.pRasterizationState = &_rasterizer;
			pipelineInfo.pMultisampleState = &_multisampling;
			pipelineInfo.pColorBlendState = &colorBlending;
			pipelineInfo.layout = _pipelineLayout;
			pipelineInfo.renderPass = pass;
			pipelineInfo.subpass = 0;
			pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

			VkPipeline newPipeline;
			if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS)
			{
				Log::PrintCore("Failed to create graphics pipeline", LogSeverity::LogError);
				return VK_NULL_HANDLE; // failed to create graphics pipeline
			}
			else
			{
				return newPipeline;
			}
		}
	};

}

VulkanPipeline::VulkanPipeline(const ShaderModule& module) : Pipeline(module)
{
	
}

bool VulkanPipeline::Build()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = static_cast<const VulkanRenderer*>(app->GetRenderer());
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return false;
	CORE_ASSERT(renderer->GetApi() == GraphicsAPI::VULKAN, "Not a vulkan instance");

	ShaderModuleArray<VkShaderModule> modules;
	LoadShaderModule(renderer->m_device, *shaderModule, modules);

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	VkPipelineBuilder pipelineBuilder;

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
	switch (primitiveTopolgy)
	{
	case PrimitiveTopolgy::POINT_LIST:
		pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
		break;
	case PrimitiveTopolgy::LINE_LIST:
		pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
		break;
	case PrimitiveTopolgy::LINE_STRIP:
		pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
		break;
	case PrimitiveTopolgy::TRIANGLE_LIST:
		pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		break;
	case PrimitiveTopolgy::TRIANGLE_STRIP:
		pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
		break;
	case PrimitiveTopolgy::TRIANGLE_FAN:
		pipelineBuilder._inputAssembly = vkinit::InputAssemblyCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
		break;
	default:
		CORE_ASSERT(false, "Topology mode not supported");
		break;
	}

	//build viewport and scissor from the swapchain extents
	pipelineBuilder._viewport.x = viewport.x;
	pipelineBuilder._viewport.y = viewport.y;
	pipelineBuilder._viewport.width = (float)viewport.w;
	pipelineBuilder._viewport.height = (float)viewport.h;
	pipelineBuilder._viewport.minDepth = viewport.minDepth;
	pipelineBuilder._viewport.maxDepth = viewport.maxDepth;

	pipelineBuilder._scissor.offset = { scissor.offsetX, scissor.offsetY };
	pipelineBuilder._scissor.extent = VkExtent2D(scissor.extentX, scissor.extentY);

	//configure the rasterizer to draw filled triangles
	switch (polygonMode)
	{
	case PolygonMode::FILL:
		pipelineBuilder._rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
		break;
	case PolygonMode::LINE:
		pipelineBuilder._rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_LINE);
		break;
	case PolygonMode::POINT:
		pipelineBuilder._rasterizer = vkinit::RasterizationStateCreateInfo(VK_POLYGON_MODE_POINT);
		break;
	default:
		CORE_ASSERT(false, "Polygon mode not supported");
		break;
	}

	//we don't use multisampling, so just run the default one
	pipelineBuilder._multisampling = vkinit::MultisamplingStateCreateInfo();

	//a single blend attachment with no blending and writing to RGBA
	pipelineBuilder._colorBlendAttachment = vkinit::ColorBlendAttachmentState();

	//use the triangle layout we created
	//pipelineBuilder._pipelineLayout = m_pipeline;

	//finally build the pipeline
	m_pipeline = pipelineBuilder.BuildPipeline(renderer->m_device, renderer->m_renderPass);

	//finally destroy shader modules
	if (vertexModule)
		vkDestroyShaderModule(renderer->m_device, vertexModule, nullptr);
	if (fragmentModule)
		vkDestroyShaderModule(renderer->m_device, fragmentModule, nullptr);


	m_deletionQueue.push_function([=]() {
		vkDestroyPipeline(renderer->m_device, m_pipeline, nullptr);
	});

	return m_pipeline != VK_NULL_HANDLE;
}

VulkanPipeline::~VulkanPipeline()
{
	m_deletionQueue.flush();
}

