#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <set>
#include <string>

#include "Utils.h"

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :
	
	//Would like this to be parametrable !
	const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::set<std::string> mRequiredExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; //std::set :(

	VkInstance		mVKInstance;
	VkDevice		mLogicalDevice;
	VkSurfaceKHR	mRenderingSurface;
	VkQueue			mPresentQueue;
	VkSwapchainKHR	mSwapChain;

	PhysicalDeviceDescription mPhysicalDevice;

private :
	bool CreateVKInstance();
	bool CanEnableValidationLayers(const std::vector<const char*>& p_validationLayers);
	bool PickPhysicalDevice();

	bool CheckDeviceExtentions(const VkPhysicalDevice& p_device);
	bool DeviceIsSupported(const VkPhysicalDevice& p_device);
	DeviceSupportedQueues GetDeviceSupportedQueues(const VkPhysicalDevice& p_device);

	//TODO: Better way to select gpu ! VKRenderer::GetBestDevice
	//TODO : VKRenderer::GetBestDevice Crash the program if no suiatable gpu
	PhysicalDeviceDescription GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices);

	//TODO : VKRenderer::CreateLogicalDevice vulkan complain both queues have the same index, but for now whatever
	bool CreateLogicalDevice();
	bool CreateSwapChain();

//
//IRenderer implementation
//

public:
	bool Init(const Window* p_window) override;
	void Release() override;
};