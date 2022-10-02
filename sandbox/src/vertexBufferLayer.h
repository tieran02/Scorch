#pragma once
#include "scorch/engine.h"

class VertexBufferLayer : public SC::Layer
{
public:
	VertexBufferLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(SC::Event& event) override;
private:
	std::unique_ptr<SC::PipelineLayout> m_pipelineLayout;
	std::unique_ptr<SC::Pipeline> m_pipeline;
	std::unique_ptr<SC::Buffer> m_vertexBuffer;
	float m_rotation;
};

