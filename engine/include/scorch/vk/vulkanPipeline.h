#pragma once
#include "render/pipeline.h"
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

		VkPipeline GetPipeline() const;
	private:
		VkPipeline m_pipeline;
		VkPipelineLayout m_tempPipelineLayout; //For now each pipeline has its own layout, in the future we need to cache and reuse layouts that are the same

		DeletionQueue m_deletionQueue;
	};
}