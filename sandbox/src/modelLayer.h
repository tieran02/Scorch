#pragma once
#include "scorch/engine.h"

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
	std::unique_ptr<SC::Buffer> m_vertexBuffer;
	SC::Mesh m_monkeyMesh;
	float m_rotation;
};

