#pragma once

#include "Window.h"

class IRenderer
{
protected:
	Window* mRenderingWindow;

public:
	virtual bool Init(Window* p_window);
	virtual void Release() = 0;
	virtual void Render()  = 0;
};