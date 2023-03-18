#pragma once
#include "scorch/engine.h"

class DeferredLayer : public SC::Layer
{
public:
	DeferredLayer() : Layer("DeferredLayer") {}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
private:
	std::unique_ptr<SC::PipelineLayout> m_pipelineLayout;
	std::unique_ptr<SC::Pipeline> m_pipeline;

	std::unique_ptr<SC::Renderpass> m_deferredRenderpass;
	std::unique_ptr<SC::RenderTarget> m_colourTarget;
};

