#include "pch.h"
#include "core/app.h"
#include "GLFW/glfw3.h"
#include "core/Renderer.h"

namespace
{
	App* g_instance{nullptr};
}

const App* App::Instance()
{
	return g_instance;
}

std::unique_ptr<App> App::Create(const std::string& title, int width, int height)
{
	assert(!g_instance); //Only one instance of app is currently supported
	if (g_instance)
		return nullptr;

	//Create a new app unique ptr and create a window
	auto app = std::unique_ptr<App>(new App(width, height));
	app->InitWindow(title);
	g_instance = app.get();

	app->InitRenderer(GraphicsAPI::VULKAN);

	return std::move(app);
}

App::App(int width, int height) :
	m_window(nullptr),
	m_width(width),
	m_height(height),
	m_isRunning(false),
	m_renderer(nullptr)
{

}

App::~App()
{
	glfwTerminate();
	glfwDestroyWindow(m_window);
}

void App::Run()
{
	m_isRunning = true;

	while (m_isRunning)
	{
		glfwPollEvents();

		m_isRunning = !glfwWindowShouldClose(m_window);
	}
}

bool App::InitWindow(const std::string& title)
{
	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(m_width, m_height, title.c_str(), NULL, NULL);
	if (!m_window)
		return false;

	return true;
}

bool App::InitRenderer(GraphicsAPI api)
{
	m_renderer = Renderer::Create(api);
	return m_renderer.get();
}

GLFWwindow* App::GetWindowHandle() const
{
	return m_window;
}
