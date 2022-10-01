#pragma once
#include "scorch/engine.h"

class VertexBufferLayer : public SC::Layer
{
public:
	VertexBufferLayer() : Layer("VertexBufferLayer") {}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate() override;
	void OnEvent(SC::Event& event) override;
private:
	std::unique_ptr<SC::Pipeline> m_pipeline;
	std::unique_ptr<SC::Buffer> m_vertexBuffer;
};

