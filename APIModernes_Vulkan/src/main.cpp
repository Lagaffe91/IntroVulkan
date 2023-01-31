#include "Window.h"

#include <GLFW/glfw3.h>

#include <iostream>

#define APP_NAME "API Graphiques modernes : vulkan"
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //for vulkan stuff

    Window* window = Window::Create(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);

    int _;
    std::cin >> _;

    window->Destroy();

    glfwTerminate();

	return 0;
}