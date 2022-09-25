#pragma once
#include "core/pipeline.h"

namespace SC 
{
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const ShaderModule& module);
	};
}