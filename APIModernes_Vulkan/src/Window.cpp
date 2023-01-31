#include "Window.h"

Window::Window(GLFWwindow* window)
{
	this->mWindow = window;
}

Window::~Window()
{
	//to stay safe
	if (this->mShouldBeDestroyed)
		this->Destroy();
}

Window* Window::Create(const std::string& p_windowName, const int& p_width, const int& p_height)
{	
	GLFWwindow* window = glfwCreateWindow(p_width, p_height, p_windowName.c_str(), nullptr, nullptr);

	if (window)
		return new Window(window);
	else
		return nullptr;
}

void Window::Destroy()
{
	glfwDestroyWindow(this->mWindow);
	
	delete this;
}
