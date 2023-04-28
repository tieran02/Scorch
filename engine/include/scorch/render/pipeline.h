#pragma once
#include "shaderModule.h"

namespace SC
{
	enum class Format : uint16_t
	{
		UNDEFINED,
		D32_SFLOAT,
		R8_SRGB,
		R8G8_SRGB,
		R8G8B8_SRGB,
		R8G8B8A8_SRGB,
		B8G8R8A8_SRGB,
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

	enum class FaceCulling
	{
		FRONT,
		BACK,
		FRONT_BACK,
		NONE
	};

	enum class ColorComponentBits : uint8_t
	{
		COLOR_COMPONENT_R,
		COLOR_COMPONENT_G,
		COLOR_COMPONENT_B,
		COLOR_COMPONENT_A,
		COUNT
	};
	using ColorComponentFlags = Flags<ColorComponentBits>;

	struct ColourBlendAttachment
	{
		ColorComponentFlags components;
		bool enableBlend;
	};

	struct PushConstant
	{
		ShaderModuleFlags shaderStages;
		uint32_t size;
	};

	class DescriptorSetLayout;
	class DescriptorSet;
	class Renderpass;
	class PipelineLayout
	{
		//This will be used to hold information on push constants/uniforms and other set locations

		//build the pipeline layout that controls the inputs/outputs of the shader
		//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default

	public:
		static std::unique_ptr<PipelineLayout> Create();
		virtual ~PipelineLayout();

		void AddColourBlendAttachment(ColorComponentFlags components, bool enableBlend);
		const std::vector<ColourBlendAttachment>& ColourBlendAttachments() const;

		void AddPushConstant(ShaderModuleFlags stages, uint32_t size);
		const std::vector<PushConstant>& PushConstants() const;

		void AddDescriptorSetLayout(const DescriptorSetLayout* layout);
		const std::vector<const DescriptorSetLayout*>& DescriptorSetLayouts() const;

		virtual bool Build() = 0;
	protected:
		PipelineLayout();

		std::vector<ColourBlendAttachment> m_colourBlendAttachments;
		std::vector<PushConstant> m_pushConstants;
		std::vector<const DescriptorSetLayout*> m_descriptorSetLayouts;
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
		FaceCulling				faceCulling;

		//TODO multi sampling and blending
	public:
		virtual bool Build(const Renderpass* renderpass = nullptr) = 0;

		virtual ~Pipeline();
	protected:
		Pipeline(const ShaderModule& module);
	};
}