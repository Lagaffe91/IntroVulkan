#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "Utils.h"

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :
	VkInstance			mVKInstance;
	VkDevice			mLogicalDevice;

	PhysicalDeviceDescription mPhysicalDevice;

private :
	bool CreateVKInstance();
	bool CanEnableValidationLayers(const std::vector<const char*>& p_validationLayers);
	bool PickPhysicalDevice();

	PhysicalDeviceDescription GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices);

	bool CreateLogicalDevice();

//
//IRenderer implementation
//

public:
	bool Init() override;
	void Release() override;
};