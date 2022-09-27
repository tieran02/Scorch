#pragma once
#include "core/layer.h"
struct GLFWwindow;

namespace SC 
{
	class Event;
	class WindowCloseEvent;
	class WindowResizeEvent;
	class Renderer;
	enum class GraphicsAPI;
	class App
	{
	public:
		static const App* Instance();
	public:
		static std::unique_ptr<App> Create(const std::string& title, int width, int height);
		~App();

		void Run();
		void Close();

		void OnEvent(Event& e);
		void PushLayer(std::shared_ptr<Layer>& layer);
		void PushOverlay(std::shared_ptr<Layer>& overlay);

		GLFWwindow* GetWindowHandle() const;
		void GetWindowExtent(int& width, int& height) const;
		double GetWindowTime() const;

		const Renderer* GetRenderer() const;
	private:
		App(int width, int height);
		bool InitWindow(const std::string& title);
		bool InitRenderer(GraphicsAPI api);

		bool OnWindowClose(WindowCloseEvent e);
		bool OnWindowResize(WindowResizeEvent e);
	private:
		int m_width, m_height;
		GLFWwindow* m_window;

		std::unique_ptr<Renderer> m_renderer;

		LayerStack m_layerStack;
		bool m_isRunning;
	};
}