#include "pch.h"
#include "render/commandbuffer.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanCommandbuffer.h"

using namespace SC;

std::unique_ptr<CommandPool> SC::CommandPool::Create()
{
	SCORCH_API_CREATE(CommandPool);
}
