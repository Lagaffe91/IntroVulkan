#include "IRenderer.h"


bool IRenderer::Init(Window* p_window)
{
	this->mRenderingWindow = p_window;
	
	return p_window != nullptr;
}