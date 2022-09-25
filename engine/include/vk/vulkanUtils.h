#include "volk.h"
#include "core/shaderModule.h"

namespace SC 
{
	bool LoadShaderModule(VkDevice device, const ShaderModule& module, ShaderModuleArray<VkShaderModule>& outShaderModules);

	class PipelineBuilder
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

		VkPipeline BuildPipeline(VkDevice device, VkRenderPass pass);
	};
}
