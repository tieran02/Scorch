#include "pch.h"
#include "core/app.h"
#include "GLFW/glfw3.h"
#include "core/Renderer.h"
#include "event/Event.h"
#include "event/ApplicationEvent.h"
#include "event/KeyEvent.h"
#include "event/MouseEvent.h"

using namespace SC;

namespace
{
	App* g_instance{nullptr};

	using EventCallbackFn = std::function<void(Event&)>;
	EventCallbackFn EventCallback;
}

const App* App::Instance()
{
	return g_instance;
}

std::unique_ptr<App> App::Create(const std::string& title, int width, int height)
{
	Log::PrintCore(string_format("Creating App (width %i, height %i)", width, height));

	CORE_ASSERT(!g_instance, "Only one instance of app is currently supported");
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
	m_renderer.reset();

	Log::PrintCore("Destroying window");
	glfwTerminate();
	glfwDestroyWindow(m_window);
}

void App::Run()
{
	m_isRunning = true;

	while (m_isRunning)
	{
		glfwPollEvents();

		for (auto& layer : m_layerStack)
		{
			layer->OnUpdate();
			if (!m_isRunning) break;
		}

		if (m_renderer)
			m_renderer->Draw();
	}
}

void App::Close()
{
	m_isRunning = false;

	for (auto& layer : m_layerStack)
		layer->OnDetach();
	m_layerStack.DeleteLayers();
}

void App::OnEvent(Event& e)
{
	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<WindowCloseEvent>(std::bind(&App::OnWindowClose, this, std::placeholders::_1));

	dispatcher.Dispatch<WindowResizeEvent>(std::bind(&App::OnWindowResize, this, std::placeholders::_1));

	if (e.GetEventType() == EventType::KeyReleased && static_cast<KeyReleaseEvent&>(e).GetKeyCode() == 256) //escape key
		Close();

	//handle layer events in reverse
	for (auto it = m_layerStack.end(); it != m_layerStack.begin(); )
	{
		(*--it)->OnEvent(e);
		if (e.Handled)
			break;
	}
}

void App::PushLayer(std::shared_ptr<Layer>& layer)
{
	m_layerStack.PushLayer(layer);
	layer->OnAttach();
}

void App::PushOverlay(std::shared_ptr<Layer>& overlay)
{
	m_layerStack.PushOverlay(overlay);
	overlay->OnAttach();
}

bool App::InitWindow(const std::string& title)
{
	Log::PrintCore("Creating GLFW Window");

	if (!glfwInit())
		return false;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(m_width, m_height, title.c_str(), NULL, NULL);
	CORE_ASSERT(m_window, "Failed to create GLFW Window");
	if (!m_window)
		return false;

	// Set GLFW callbacks
	glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
		{
			WindowResizeEvent event(width, height);
			EventCallback(event);
		});

	glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
		{
			WindowCloseEvent event;
			EventCallback(event);
		});

	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(key, 0);
				EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleaseEvent event(key);
				EventCallback(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyPressedEvent event(key, 1);
				EventCallback(event);
				break;
			}
			}
		});

	glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int keycode)
		{
			KeyTypedEvent event(keycode);
			EventCallback(event);
		});

	glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
		{
			switch (action)
			{
			case GLFW_PRESS:
			{
				MouseButtonPressedEvent event(button);
				EventCallback(event);
				break;
			}
			case GLFW_RELEASE:
			{
				MouseButtonReleasedEvent event(button);
				EventCallback(event);
				break;
			}
			}
		});

	glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
		{
			MouseScrolledEvent event((float)xOffset, (float)yOffset);
			EventCallback(event);
		});

	glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
		{
			MouseMovedEvent event((float)xPos, (float)yPos);
			EventCallback(event);
		});

	EventCallback = std::bind(&App::OnEvent, this, std::placeholders::_1);

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

void App::GetWindowExtent(int& width, int& height) const
{
	width = m_width;
	height = m_height;
}

double App::GetWindowTime() const
{
	return glfwGetTime();
}

const Renderer* App::GetRenderer() const
{
	return m_renderer.get();
}

bool App::OnWindowClose(WindowCloseEvent e)
{
	Close();
	return true;
}

bool App::OnWindowResize(WindowResizeEvent e)
{
	return true;
}
