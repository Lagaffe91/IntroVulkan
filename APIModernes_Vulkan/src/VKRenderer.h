#pragma once

#include "vulkan/vulkan.h"

#include <vector>

#include "Utils.h"

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :
	
	const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" }; //More ?

	VkInstance			mVKInstance;
	VkDevice			mLogicalDevice;
	VkSurfaceKHR		mRenderingSurface;
	VkQueue				mPresentQueue;

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
	bool Init(const Window* p_window) override;
	void Release() override;
};