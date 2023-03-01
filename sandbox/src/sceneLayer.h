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
	SC::FrameData<SC::Buffer> m_cameraBuffer;

	SC::MaterialSystem m_materialSystem;
	SC::ShaderEffect m_shaderEffect;
	SC::ShaderPass m_shaderPass;
	SC::Scene m_scene;

	SC::SceneNode* helmetRoot;
	SC::SceneNode* sponzaRoot;

	float m_rotation;
};

