#include "pch.h"
#include "vk/vulkanPipeline.h"
#include "render/shaderModule.h"
#include "core/app.h"
#include "vk/vulkanRenderer.h"
#include "vk/vulkanInitialiser.h"
#include "vk/vulkanUtils.h"
#include "vk/vulkanDescriptorSet.h"

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

	void LoadVertexInputInfo(const VertexInputDescription& vertexInput, std::vector<VkVertexInputBindingDescription>& bindings, std::vector<VkVertexInputAttributeDescription>& attributes)
	{
		//only support one binding for now
		VkVertexInputBindingDescription mainBinding = {};
		mainBinding.binding = 0;
		mainBinding.stride = vertexInput.GetStride();
		switch (vertexInput.InputRate())
		{
		case VertexInputRate::VERTEX:
			mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			break;
		case VertexInputRate::INSTANCE:
			mainBinding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			break;
		default:
			CORE_ASSERT(false, "Input rate not supported");
			return;
		}
		bindings.push_back(mainBinding);

		attributes.resize(vertexInput.Attributes().size());
		uint32_t offset = 0;
		for (int i = 0; i < vertexInput.Attributes().size(); ++i)
		{
			attributes[i].binding = 0;
			attributes[i].format = vkutils::ConvertFormat(vertexInput.Attributes()[i]);
			attributes[i].location = i;
			attributes[i].offset = offset;

			offset += vertexInput.GetAttributeSize(i);
		}
	}

	class VkPipelineBuilder
	{
	public:
		std::vector<VkPipelineShaderStageCreateInfo> _shaderStages{};
		VkPipelineVertexInputStateCreateInfo _vertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo _inputAssembly{};
		VkViewport _viewport{};
		VkRect2D _scissor{};
		VkPipelineRasterizationStateCreateInfo _rasterizer{};
		VkPipelineColorBlendAttachmentState _colorBlendAttachment{};
		VkPipelineMultisampleStateCreateInfo _multisampling{};
		VkPipelineLayout _pipelineLayout{};
		VkPipelineDepthStencilStateCreateInfo _depthStencil;

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

			pipelineInfo.stageCount = static_cast<uint32_t>(_shaderStages.size());
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

			pipelineInfo.pDepthStencilState = &_depthStencil;

			std::vector<VkDynamicState> dynamicStates = 
			{
				VK_DYNAMIC_STATE_VIEWPORT,
				VK_DYNAMIC_STATE_SCISSOR
			};
			VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
			dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
			dynamicStateInfo.pDynamicStates = dynamicStates.data();

			pipelineInfo.pDynamicState = &dynamicStateInfo;

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


VulkanPipelineLayout::VulkanPipelineLayout() : PipelineLayout()
{

}

VulkanPipelineLayout::~VulkanPipelineLayout()
{
	m_deletionQueue.flush();
}

bool VulkanPipelineLayout::Build()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = vkinit::PipelineLayoutCreateInfo();

	std::vector<VkPushConstantRange> pushConstants(m_pushConstants.size());
	uint32_t offset = 0;
	for (int i = 0; i < m_pushConstants.size(); ++i)
	{
		//this push constant range starts at the beginning
		pushConstants[i].offset = offset;
		//this push constant range takes up the size of a MeshPushConstants struct
		pushConstants[i].size = m_pushConstants[i].size;
		//this push constant range is accessible only in the vertex shader
		pushConstants[i].stageFlags = 0;
		if(m_pushConstants[i].shaderStages.test(ShaderStage::VERTEX))
			pushConstants[i].stageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
		if (m_pushConstants[i].shaderStages.test(ShaderStage::FRAGMENT))
			pushConstants[i].stageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;

		offset += pushConstants[i].size;
	}
	pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
	pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());

	std::vector<VkDescriptorSetLayout> setLayouts(m_descriptorSetLayouts.size());
	for (int i = 0; i < m_descriptorSetLayouts.size(); ++i)
	{
		const VulkanDescriptorSetLayout* layout = static_cast<const VulkanDescriptorSetLayout*>(m_descriptorSetLayouts[i]);
		setLayouts[i] = layout->m_layout;
	}
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());


	VK_CHECK(vkCreatePipelineLayout(renderer->m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkDestroyPipelineLayout(renderer->m_device, m_pipelineLayout, nullptr);
		});

	return true;
}

VkPipelineLayout VulkanPipelineLayout::GetPipelineLayout() const
{
	return m_pipelineLayout;
}

VulkanPipeline::VulkanPipeline(const ShaderModule& module) : Pipeline(module),
	m_pipeline(VK_NULL_HANDLE),
	m_tempPipelineLayout(VK_NULL_HANDLE)
{
	
}

bool VulkanPipeline::Build()
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return false;

	const VulkanRenderer* renderer = app->GetVulkanRenderer();
	if (!renderer)
		return false;


	ShaderModuleArray<VkShaderModule> modules;
	LoadShaderModule(renderer->m_device, *shaderModule, modules);

	//build the stage-create-info for both vertex and fragment stages. This lets the pipeline know the shader modules per stage
	VkPipelineBuilder pipelineBuilder;

	//set the pipeline layout or create a empty layout if none is selected
	if (!pipelineLayout)
	{
		//No pipeline set so create an empty layout
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkinit::PipelineLayoutCreateInfo();
		VK_CHECK(vkCreatePipelineLayout(renderer->m_device, &pipeline_layout_info, nullptr, &m_tempPipelineLayout));
		m_deletionQueue.push_function([=]() {
			renderer->WaitOnFences();
			vkDestroyPipelineLayout(renderer->m_device, m_tempPipelineLayout, nullptr);
			});

		//use temp layout for nw
		pipelineBuilder._pipelineLayout = m_tempPipelineLayout;
	}
	else
	{
		pipelineBuilder._pipelineLayout = static_cast<const VulkanPipelineLayout*>(pipelineLayout)->GetPipelineLayout();
	}

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

	//load vertex input
	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;
	LoadVertexInputInfo(vertexInputDescription, bindings, attributes);
	if (!bindings.empty())
	{
		pipelineBuilder._vertexInputInfo.pVertexBindingDescriptions = bindings.data();
		pipelineBuilder._vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
		pipelineBuilder._vertexInputInfo.pVertexAttributeDescriptions = attributes.data();
		pipelineBuilder._vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
	}

	//default depthtesting
	pipelineBuilder._depthStencil = vkinit::DepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);

	//finally build the pipeline
	m_pipeline = pipelineBuilder.BuildPipeline(renderer->m_device, renderer->m_renderPass);



	//finally destroy shader modules
	if (vertexModule)
		vkDestroyShaderModule(renderer->m_device, vertexModule, nullptr);
	if (fragmentModule)
		vkDestroyShaderModule(renderer->m_device, fragmentModule, nullptr);


	m_deletionQueue.push_function([=]() {
		renderer->WaitOnFences();
		vkDestroyPipeline(renderer->m_device, m_pipeline, nullptr);
	});

	return m_pipeline != VK_NULL_HANDLE;
}

VulkanPipeline::~VulkanPipeline()
{
	m_deletionQueue.flush();
}

VkPipeline VulkanPipeline::GetPipeline() const
{
	return m_pipeline;
}
