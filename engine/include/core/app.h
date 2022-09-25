#pragma once
struct GLFWwindow;
namespace SC 
{
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

		GLFWwindow* GetWindowHandle() const;
		void GetWindowExtent(int& width, int& height) const;
		double GetWindowTime() const;

		const Renderer* GetRenderer() const;
	private:
		App(int width, int height);
		bool InitWindow(const std::string& title);
		bool InitRenderer(GraphicsAPI api);
	private:
		int m_width, m_height;
		GLFWwindow* m_window;

		std::unique_ptr<Renderer> m_renderer;

		bool m_isRunning;
	};
}