#pragma once

#include "Engine.h"
#include "Window.h"
#include "VKRenderer.h"

#include <string>

class Application
{
private :
	static Application* mInstance;

	Window* mWindow = nullptr;
	Engine* mEngine = nullptr;

	IRenderer* mRenderer = nullptr;

private:
	bool Init(const std::string& p_windowName, const int& p_width, const int& p_height);
	void Release();
	void Render();

public :
	static void Create();
	static void Destroy();
	static Application& Get();
	
	int Run(const std::string& p_windowName, const int& p_width, const int& p_height);
	void Quit();
};

