#pragma once
#include "scorch/engine.h"

class MaterialLayer : public SC::Layer
{
public:
	MaterialLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(SC::Event& event) override;
private:
	void CreateScene();
	void Draw();
private:
	std::unique_ptr<SC::Pipeline> m_pipeline;

	SC::ShaderEffect m_shaderEffect;

	SC::FrameData<SC::Buffer> m_cameraBuffer;
	SC::FrameData<SC::DescriptorSet> m_globalDescriptorSet;

	SC::Camera m_camera;
	SC::Scene m_scene;

	float m_rotation;
	glm::vec3 m_pos;

	std::unordered_map<std::string, std::unique_ptr<SC::Texture>> m_textures;
};

