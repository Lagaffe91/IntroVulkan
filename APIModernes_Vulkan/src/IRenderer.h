#pragma once

class IRenderer
{
public:
	virtual bool Init() = 0;
	virtual void Release() = 0;
};