#pragma once

class App
{
public:
	static std::unique_ptr<App> Create(const std::string& title, int width, int height);
	~App();

	void Run();
private:
	App(int width, int height);
	bool InitWindow(const std::string& title);
private:
	int m_width, m_height;
	struct GLFWwindow* m_window;

	std::unique_ptr<class Renderer> m_renderer;

	bool m_isRunning;
};