#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :
	VkInstance			mVKInstance;
	VkPhysicalDevice	mPhysicalDevice;
	VkDevice			mLogicalDevice;

private :
	bool CreateVKInstance();
	bool CanEnableValidationLayers(const std::vector<const char*>& p_validationLayers, uint32_t* p_layerCount);
	bool PickPhysicalDevice();

	VkPhysicalDevice GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices);

	bool CreateLogicalDevice();

//
//IRenderer implementation
//

public:
	bool Init() override;
	void Release() override;
};