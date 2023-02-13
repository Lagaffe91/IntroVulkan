#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <string>

#include "Utils.h"

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :

	//Would like this to be parametrable ?
	const std::vector<const char*> mValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> mExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkInstance		mVKInstance;
	VkDevice		mLogicalDevice;
	VkSurfaceKHR	mRenderingSurface;

	VkQueue			mPresentQueue;
	VkQueue			mGraphicsQueue;

	PhysicalDeviceDescription	mPhysicalDevice;
	SwapChainDescription		mSwapChain;


	VkShaderModule mVertexShader;
	VkShaderModule mFragmentShader;

	//TODO : Put render pipeline objects in a sturct !
	std::vector<VkPipelineShaderStageCreateInfo> mShaderStages;

	VkPipelineLayout mPipelineLayout;
	VkRenderPass mRenderPass;
	VkPipeline mGraphicsPipeline;

	//

	std::vector<VkFramebuffer> mFrameBuffers;

	VkCommandPool	mCommandPool;
	VkCommandBuffer mCommandBuffer;

private :
	bool CreateVKInstance();
	bool CanEnableValidationLayers(const std::vector<const char*>& p_validationLayers);
	bool PickPhysicalDevice();

	bool CheckDeviceExtentions(const VkPhysicalDevice& p_device);
	bool DeviceIsSupported(const VkPhysicalDevice& p_device);

	DeviceSupportedQueues GetDeviceSupportedQueues(const VkPhysicalDevice& p_device);
	SwapChainCapabilities GetSwapChainParameters(const VkPhysicalDevice& p_device);
	//TODO: Better way to select gpu ! VKRenderer::GetBestDevice
	//TODO : VKRenderer::GetBestDevice Crash the program if no suiatable gpu
	PhysicalDeviceDescription GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices);

	//TODO : VKRenderer::CreateLogicalDevice : Vulkan complain both queues have the same index, but for now whatever
	bool CreateLogicalDevice();

	VkExtent2D GetSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	//TODO : VKRenderer::CreateLogicalDevice : presentMode is hardcoded to FIFO (i dont want anything else, but could be cool to make it parametrable)
	bool CreateSwapChain();
	
	bool SetupGraphicsPipeline();

	bool CreateFrameBuffers();

	bool CreateCommandBuffer();

	void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

public:
	VkShaderModule LoadShader(const std::vector<char>& p_byteCode); //TODO : put LoadShader() in a Ressource manager or smth


//
//IRenderer implementation
//

public:
	bool Init(Window* p_window) override;
	void Release() override;
};