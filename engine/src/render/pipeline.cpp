#include "pch.h"
#include "render/pipeline.h"
#include "render/shaderModule.h"
#include "core/app.h"
#include "render/renderer.h"
#include "vk/vulkanPipeline.h"

using namespace SC;

VertexInputBindingDescription::VertexInputBindingDescription() :
	binding{ 0 },
	stride{ 0 },
	inputRate{ VertexInputRate::VERTEX }
{

}

VertexInputAttributeDescription::VertexInputAttributeDescription() :
	location { 0 },
	binding{ 0 },
	format{ Format::UNDEFINED },
	offset{ 0 }
{

}

Viewport::Viewport() : 
	x{ 0 },
	y{ 0 },
	w{ 0 },
	h{ 0 },
	minDepth(0.0f),
	maxDepth(1.0f)
{

}

Scissor::Scissor() :
	offsetX{ 0 },
	offsetY{ 0 },
	extentX{ 0 },
	extentY{ 0 }
{

}


std::unique_ptr<Pipeline> Pipeline::Create(const ShaderModule& module)
{
	const App* app = App::Instance();
	CORE_ASSERT(app, "App instance is null");
	if (!app) return nullptr;
	
	const Renderer* renderer = app->GetRenderer();
	CORE_ASSERT(renderer, "renderer is null");
	if (!renderer) return nullptr;

	std::unique_ptr<Pipeline> pipeline{nullptr};
	switch (renderer->GetApi())
	{
	case GraphicsAPI::VULKAN:
		pipeline = std::unique_ptr<VulkanPipeline>(new VulkanPipeline(module));
	}

	CORE_ASSERT(pipeline, "failed to create pipeline");
	return std::move(pipeline);
}

Pipeline::Pipeline(const ShaderModule& module) :
	shaderModule(&module),
	pipelineLayout(nullptr),
	primitiveTopolgy(PrimitiveTopolgy::TRIANGLE_LIST),
	polygonMode(PolygonMode::FILL)
{

	//Attempt to construct the viewport given the app window as the default param
	const App* app = App::Instance();
	if (app)
	{
		int windowWidth{ 0 }, windowHeight{ 0 };
		app->GetWindowExtent(windowWidth, windowHeight);

		viewport.w = static_cast<float>(windowWidth);
		viewport.h = static_cast<float>(windowHeight);
		scissor.extentX = static_cast<uint32_t>(windowWidth);
		scissor.extentY = static_cast<uint32_t>(windowHeight);
	}
	
}

Pipeline::~Pipeline()
{

}
