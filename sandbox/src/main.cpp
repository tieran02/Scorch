#include <engine.h>

int main()
{
	SC::Log::Print("Hello game!");
	if (auto app = SC::App::Create("Hello Vulkan", 1280, 720))
	{
		SC::ShaderModuleBuilder shaderBuilder;
		auto shader = shaderBuilder.SetVertexModulePath("shaders/coloured_triangle.vert.spv")
			.SetFragmentModulePath("shaders/coloured_triangle.frag.spv")
			.Build();

		auto pipeline =  SC::Pipeline::Create(*shader);
		pipeline->Build();

		app->Run();
	}

	return 0;
}