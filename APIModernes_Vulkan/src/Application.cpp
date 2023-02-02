#include "Application.h"

#include <iostream>

Application* Application::mInstance = nullptr;

bool Application::Init(const std::string& p_windowName, const int& p_width, const int& p_height)
{
	//
	//(I'd like to move the glfw part to IRenderer or smth else !)
	//

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //For vulkan 

	//Here ?
	this->mRenderer = new VKRenderer();
	this->mWindow = Window::Create(p_windowName, p_width, p_height);
	this->mEngine = new Engine();

	this->mRenderer->Init(this->mWindow);

	return (this->mRenderer || this->mWindow || this->mEngine);
}

void Application::Release()
{
	if (mRenderer)
		mRenderer->Release();

	if (mWindow)
		mWindow->Destroy();

	if (mEngine)
		mEngine->Destroy();
}

void Application::Create()
{
	if (!Application::mInstance)
		Application::mInstance = new Application();
}

void Application::Destroy()
{
	if (Application::mInstance)
		delete Application::mInstance;
}

Application& Application::Get()
{
	return *mInstance;
}

int Application::Run(const std::string& p_windowName, const int& p_width, const int& p_height)
{
	bool init = this->Init(p_windowName, p_width, p_height);

	if (!init)
		return 1;
	
	while (!mWindow->ShouldClose()) 
	{
		glfwPollEvents();
	}

	this->Quit();

	return 0;
}

void Application::Quit()
{
	this->Release();

	glfwTerminate();
}