#pragma once
#include "shaderModule.h"
#include "descriptorSet.h"

namespace SC
{
	enum class Format : uint16_t
	{
		UNDEFINED,
		D32_SFLOAT,
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
		Viewport(float x, float y, float w, float h);
		float	x, y, w, h;
		float	minDepth, maxDepth;
	};

	struct Scissor
	{
		Scissor();
		Scissor(uint32_t extentX, uint32_t extentY);

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

	struct PushConstant
	{
		ShaderModuleFlags shaderStages;
		uint32_t size;
	};

	class DescriptorSet;
	class PipelineLayout
	{
		//This will be used to hold information on push constants/uniforms and other set locations

		//build the pipeline layout that controls the inputs/outputs of the shader
		//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default

	public:
		static std::unique_ptr<PipelineLayout> Create();
		virtual ~PipelineLayout();

		void AddPushConstant(ShaderModuleFlags stages, uint32_t size);
		const std::vector<PushConstant>& PushConstants() const;

		void AddDescriptorSetLayout(const DescriptorSetLayout& layput);
		const std::vector<DescriptorSetLayout>& DescriptorSetLayouts() const;

		virtual bool Build() = 0;
	protected:
		PipelineLayout();

		std::vector<PushConstant> m_pushConstants;
		std::vector<DescriptorSetLayout> m_descriptorSetLayouts;
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