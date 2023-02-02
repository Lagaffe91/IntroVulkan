#pragma once

#include <GLFW/glfw3.h>

#include <string>

class Window
{
public:
	Window(GLFWwindow* window);
	~Window();

private:
	std::string mWindowName = "";

	int mWidth = 0;
	int mHeight = 0;

	bool mShouldBeDestroyed = false;

public :

	GLFWwindow* mWindow = nullptr; //Read only but whatever

	static Window* Create(const std::string& p_windowName, const int& p_width, const int& p_height);
	
	void Destroy();

	bool ShouldClose();
};

