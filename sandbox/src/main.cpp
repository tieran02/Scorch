#include <scorch/engine.h>
#include "testLayer.h"
#include "vertexBufferLayer.h"
#include "modelLayer.h"
#include "descriptorLayer.h"
#include "MaterialLayer.h"

int main()
{
	SC::Log::Print("Hello game!");
	if (auto app = SC::App::Create("Hello Vulkan", 1280, 720))
	{
		//std::shared_ptr<SC::Layer> testLayer = std::make_shared<TestLayer>();
		//std::shared_ptr<SC::Layer> vertexBufferLayer = std::make_shared<VertexBufferLayer>();
		//std::shared_ptr<SC::Layer> modelLayer = std::make_shared<ModelLayer>();
		//std::shared_ptr<SC::Layer> modelLayer = std::make_shared<DescriptorLayer>();
		//std::shared_ptr<SC::Layer> modelLayer = std::make_shared<SceneLayer>();
		std::shared_ptr<SC::Layer> modelLayer = std::make_shared<MaterialLayer>();


		app->PushLayer(modelLayer);
		app->Run();
	}

	return 0;
}