#pragma once
#include "Engine.h"
#include "core/pipeline.h"


class TestLayer : public SC::Layer
{
public:
	TestLayer() : Layer("TestLayer") {}

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate() override;
	void OnEvent(SC::Event& event) override;
private:
	std::unique_ptr<SC::Pipeline> m_pipeline;
};

