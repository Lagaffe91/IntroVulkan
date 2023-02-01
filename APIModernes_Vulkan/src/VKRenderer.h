#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :
	VkInstance mVKInstance;

private :
	bool CreateVKInstance();
	bool EnableValidationLayers(const std::vector<const char*>& validationLayers, uint32_t* layerCount);

public:
	bool Init() override;
	void Release() override;
};