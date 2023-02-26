#pragma once
#include "scorch/engine.h"

//#define ModelLayer_UseMaterialSystem

class ModelLayer : public SC::Layer
{
public:
	ModelLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(SC::Event& event) override;
private:
	SC::FrameData<SC::Buffer> m_cameraBuffer;

#ifdef ModelLayer_UseMaterialSystem
	SC::MaterialSystem m_materialSystem;
	SC::ShaderEffect m_shaderEffect;
	SC::ShaderPass m_shaderPass;
#else
	std::unique_ptr<SC::PipelineLayout> m_pipelineLayout;
	std::unique_ptr<SC::Pipeline> m_pipeline;
#endif // ModelLayer_UseMaterialSystem


	std::unique_ptr<SC::Buffer> m_vertexBuffer;
	std::unique_ptr<SC::Buffer> m_indexBuffer;
	std::unique_ptr<SC::Texture> m_texture;
	SC::Mesh m_monkeyMesh;

	float m_rotation;
};

