#include <scorch/engine.h>
#include "testLayer.h"
#include "vertexBufferLayer.h"

int main()
{
	SC::Log::Print("Hello game!");
	if (auto app = SC::App::Create("Hello Vulkan", 1280, 720))
	{
		//std::shared_ptr<SC::Layer> testLayer = std::make_shared<TestLayer>();
		std::shared_ptr<SC::Layer> vertexBufferLayer = std::make_shared<VertexBufferLayer>();
		app->PushLayer(vertexBufferLayer);
		app->Run();
	}

	return 0;
}