#pragma once
struct GLFWwindow;
class App
{
public:
	static const App* Instance();
public:
	static std::unique_ptr<App> Create(const std::string& title, int width, int height);
	~App();

	void Run();

	GLFWwindow* GetWindowHandle() const;
private:
	App(int width, int height);
	bool InitWindow(const std::string& title);
	bool InitRenderer(enum class GraphicsAPI api);
private:
	int m_width, m_height;
	GLFWwindow* m_window;

	std::unique_ptr<class Renderer> m_renderer;

	bool m_isRunning;
};