#pragma once
#include "scorch/engine.h"

class SceneLayer : public SC::Layer
{
public:
	SceneLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
private:
	SC::MaterialSystem m_materialSystem;
	SC::ShaderEffect m_shaderEffect;
	SC::ShaderPass m_shaderPass;
	SC::Scene m_scene;

	std::unique_ptr<SC::GUI> m_gui;

	SC::FrameData<SC::DescriptorSet> m_sceneDescriptorSet;

	SC::SceneNode* helmetRoot;
	SC::SceneNode* sponzaRoot;

	float m_globalShiniess;
	float m_rotation;
};

