#pragma once
#include "scorch/engine.h"

#define ModelLayer_UseScene

class ModelLayer : public SC::Layer
{
public:
	ModelLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(SC::Event& event) override;
private:
	std::unique_ptr<SC::PipelineLayout> m_pipelineLayout;
	std::unique_ptr<SC::Pipeline> m_pipeline;

	SC::FrameData<SC::Buffer> m_cameraBuffer;

#ifdef ModelLayer_UseScene
	SC::Scene m_scene;
#else
	std::unique_ptr<SC::Buffer> m_vertexBuffer;
	std::unique_ptr<SC::Buffer> m_indexBuffer;
	SC::Mesh m_monkeyMesh;
#endif // ModelLayer_UseScene

	float m_rotation;
};

