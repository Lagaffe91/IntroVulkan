#include "Application.h"

#include "Utils.h"

//TODO : VkRenderer : Remove every member function that does not acces members outside the class !
//TODO : VkRenderer : Window resizeing

int main()
{
	Application::Create();

	Application app = Application::Get();

	int result = app.Run(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);

	app.Destroy();

	return result;
}