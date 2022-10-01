#pragma once

namespace SC
{
	enum class Format : uint16_t
	{
		UNDEFINED,
		R32_SFLOAT,
		R32G32_SFLOAT,
		R32G32B32_SFLOAT,
		R32G32B32A32_SFLOAT,
	};
	uint32_t ConvertFormatSize(Format format);

	enum class VertexInputRate
	{
		VERTEX,
		INSTANCE,
	};

	struct VertexInputDescription
	{
		VertexInputDescription(VertexInputRate inputRate = VertexInputRate::VERTEX);
		void PushBackAttribute(Format&& format);
		uint32_t GetStride() const;
		uint32_t GetAttributeSize(int index) const;
		uint32_t GetAttributeOffset(int index) const;
		const std::vector<Format>& Attributes() const;
		VertexInputRate InputRate() const;
	private:
		std::vector<Format> m_attributes;
		VertexInputRate m_inputRate;
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
	class Pipeline
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