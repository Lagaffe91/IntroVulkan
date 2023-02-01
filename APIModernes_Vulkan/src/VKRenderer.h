#pragma once

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
public:
	bool Init() override;
};