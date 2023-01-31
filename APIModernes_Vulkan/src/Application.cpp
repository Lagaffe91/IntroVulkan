#include "Application.h"

#include <iostream>

bool Application::Init(const std::string& p_windowName, const int& p_width, const int& p_height)
{
	if (mRenderer)
		delete mRenderer;

	if (mWindow)
		delete mWindow;

	if (mEngine)
		delete mEngine;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //prepare for vulkan (idealy put this in a renderer class)
	
	return false;
}

int Application::Run(const std::string& p_windowName, const int& p_width, const int& p_height)
{
	bool init = this->Init(p_windowName, p_width, p_height);

	if (!init)
		return 1;

	//TODO : main loop here
	int _;
	std::cin >> _;

	return 0;
}
