#pragma once

#include "Window.h"

class IRenderer
{
public:
	virtual bool Init(const Window* p_window) = 0;
	virtual void Release() = 0;
};