#pragma once
#include "render/pipeline.h"
#include "volk.h"
#include "core/utils.h"

namespace SC 
{
	class VulkanPipelineLayout : public PipelineLayout
	{
	public:
		VulkanPipelineLayout();
		~VulkanPipelineLayout();

		bool Build() override;

		VkPipelineLayout GetPipelineLayout() const;
	private:
		VkPipelineLayout m_pipelineLayout;
		DeletionQueue m_deletionQueue;
	};

	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const ShaderModule& module);
		~VulkanPipeline();

		bool Build(const Renderpass* renderpass = nullptr) override;

		VkPipeline GetPipeline() const;
	private:
		VkPipeline m_pipeline;
		VkPipelineLayout m_tempPipelineLayout; //For now each pipeline has its own layout, in the future we need to cache and reuse layouts that are the same

		DeletionQueue m_deletionQueue;
	};
}