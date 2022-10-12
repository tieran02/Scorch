#pragma once
#include "scorch/engine.h"

class DescriptorLayer : public SC::Layer
{
public:
	DescriptorLayer();

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float deltaTime) override;
	void OnEvent(SC::Event& event) override;
private:
	void Draw();
private:
	std::unique_ptr<SC::PipelineLayout> m_pipelineLayout;
	std::unique_ptr<SC::Pipeline> m_pipeline;
	std::unique_ptr<SC::Buffer> m_vertexBuffer;
	std::unique_ptr<SC::Buffer> m_indexBuffer;

	SC::FrameData<SC::Buffer> m_cameraBuffer;

	std::unique_ptr<SC::DescriptorSetLayout> m_setLayout;
	SC::FrameData<SC::DescriptorSet> m_globalDescriptorSet;
	

	SC::Mesh m_monkeyMesh;
	float m_rotation;
	glm::vec3 m_pos;
};

