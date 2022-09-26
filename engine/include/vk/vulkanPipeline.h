#pragma once
#include "core/pipeline.h"
#include "volk.h"
#include "core/utils.h"

namespace SC 
{
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const ShaderModule& module);
		~VulkanPipeline() override;

		bool Build() override;

	private:
		VkPipeline m_pipeline;

		DeletionQueue m_deletionQueue;
	};
}