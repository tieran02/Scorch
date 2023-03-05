#pragma once
#include "imgui.h"
#include "scorch/core/utils.h"

struct GLFWwindow;

namespace SC
{
	class Renderer;
	class GUI
	{
	public:
		static std::unique_ptr<GUI> Create(Renderer* renderer, GLFWwindow* window);
		~GUI();

		void BeginFrame();
		void EndFrame();
	private:
		GUI(Renderer* renderer, GLFWwindow* window);
		void Init();
		void Cleanup();
	private:
		Renderer* m_renderer;
		GLFWwindow* m_window;
		DeletionQueue m_deletionQueue;
	};
}