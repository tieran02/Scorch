#pragma once
#include "render/renderpass.h"
#include <volk.h>
#include "core/utils.h"

namespace SC
{
	class VulkanRenderpass : public Renderpass
	{
	public:
		VulkanRenderpass();
		~VulkanRenderpass();

		bool Build() override;

		VkRenderPass GetRenderPass() const;
	private:
		VkRenderPass m_renderpass;
		DeletionQueue m_deletionQueue;
	};
}