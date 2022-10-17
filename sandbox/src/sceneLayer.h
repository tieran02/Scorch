#pragma once
#include "scorch/engine.h"

class SceneLayer : public SC::Layer
{
public:
	SceneLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(SC::Event& event) override;
private:
	void CreateScene();
	void Draw();
private:
	std::unique_ptr<SC::PipelineLayout> m_pipelineLayout;
	std::unique_ptr<SC::Pipeline> m_pipeline;

	SC::FrameData<SC::Buffer> m_cameraBuffer;

	std::unique_ptr<SC::DescriptorSetLayout> m_setLayout;
	SC::FrameData<SC::DescriptorSet> m_globalDescriptorSet;

	SC::Camera m_camera;
	SC::Scene m_scene;

	float m_rotation;
	glm::vec3 m_pos;

	std::unique_ptr<SC::Texture> m_testTexture;
};

