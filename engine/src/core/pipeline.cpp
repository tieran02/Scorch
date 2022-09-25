#include "pch.h"
#include "core/pipeline.h"
#include "core/shaderModule.h"

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

Pipeline::Pipeline(const ShaderModule& module) :
	shaderModule(&module),
	primitiveTopolgy(PrimitiveTopolgy::TRIANGLE_LIST),
	polygonMode(PolygonMode::FILL)
{

}
