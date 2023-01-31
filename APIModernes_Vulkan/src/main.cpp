#include "Application.h"

#define APP_NAME "API Graphiques modernes : vulkan"
#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480

int main()
{
	Application::Create();

	Application app = Application::Get();

	return app.Run(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);
}