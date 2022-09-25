#include "pch.h"
#include "vk/vulkanUtils.h"

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
}

bool SC::LoadShaderModule(VkDevice device, const ShaderModule& module, ShaderModuleArray<VkShaderModule>& outShaderModules)
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

VkPipeline PipelineBuilder::BuildPipeline(VkDevice device, VkRenderPass pass)
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
