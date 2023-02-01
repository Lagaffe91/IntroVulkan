#include "Application.h"

#include "Utils.h"

int main()
{
	Application::Create();

	Application app = Application::Get();

	int result = app.Run(APP_NAME, WINDOW_WIDTH, WINDOW_HEIGHT);

	app.Destroy();

	return result;
}