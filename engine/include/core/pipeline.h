#pragma once

namespace SC
{

	enum class Format
	{
		UNDEFINED,
		R32G32B32_SFLOAT,
	};

	enum class VertexInputRate
	{
		VERTEX,
		INSTANCE,
	};

	struct VertexInputBindingDescription
	{
		VertexInputBindingDescription();
		uint32_t		binding;
		uint32_t		stride;
		VertexInputRate	inputRate;
	};

	struct VertexInputAttributeDescription
	{
		VertexInputAttributeDescription();
		uint32_t	location;
		uint32_t	binding;
		Format		format;
		uint32_t	offset;
	};

	struct VertexInputDescription
	{
		std::vector<VertexInputBindingDescription> bindings;
		std::vector<VertexInputAttributeDescription> attributes;
	};

	struct Viewport
	{
		Viewport();
		float	x, y, w, h;
		float	minDepth, maxDepth;
	};

	struct Scissor
	{
		Scissor();
		int32_t offsetX, offsetY;
		uint32_t extentX, extentY;
	};

	enum class PrimitiveTopolgy
	{
		POINT_LIST,
		LINE_LIST,
		LINE_STRIP,
		TRIANGLE_LIST,
		TRIANGLE_STRIP,
		TRIANGLE_FAN
	};

	enum class PolygonMode
	{
		FILL,
		LINE,
		POINT
	};

	struct PipelineLayout
	{
		//This will be used to hold information on push constants/uniforms and other set locations

		//build the pipeline layout that controls the inputs/outputs of the shader
		//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
	};

	class ShaderModule;
	struct Pipeline
	{
	public:
		static std::unique_ptr<Pipeline> Create(const ShaderModule& module);

		const PipelineLayout*	pipelineLayout;
		const ShaderModule*		shaderModule;
		VertexInputDescription	vertexInputDescription;
		PrimitiveTopolgy		primitiveTopolgy;
		PolygonMode				polygonMode;
		Viewport				viewport;
		Scissor					scissor;

		//TODO multi sampling and blending
	public:
		virtual bool Build() = 0;

		virtual ~Pipeline();
	protected:
		Pipeline(const ShaderModule& module);
	};
}