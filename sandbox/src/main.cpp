#include <iostream>
#include "core/app.h"

int main()
{
	if(auto app = App::Create("Hello Vulkan", 1280, 720))
		app->Run();

	return 0;
}