#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <array>

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
	GraphicPipelineDescription  mGraphicsPipeline;

	VkShaderModule mVertexShader;
	VkShaderModule mFragmentShader;

	//TODO : Put this in a dedicated struct
	VkCommandPool	mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffer;
	//------

	//Same
	std::vector<VkSemaphore>	mRenderingSemaphore;
	std::vector<VkSemaphore>	mImageAviableSemaphore;
	std::vector<VkFence>		mPresentFence;
	//------

	uint32_t mCurrentFrame = 0;

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

	void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

	bool CreateSyncObjects();

public:
	VkShaderModule LoadShader(const std::vector<char>& p_byteCode); //TODO : put LoadShader() in a Ressource manager or smth

	static VkVertexInputBindingDescription GetBindingDescription()
	{	
		VkVertexInputBindingDescription vertexInputBindingDescription{};

		vertexInputBindingDescription.binding	= 0;
		vertexInputBindingDescription.stride	= sizeof(Vertex);
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return vertexInputBindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributeDescription{};

		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[0].offset = offsetof(Vertex, Vertex::pos);


		vertexInputAttributeDescription[1].binding = 0;
		vertexInputAttributeDescription[1].location = 1;
		vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[1].offset = offsetof(Vertex, Vertex::color);

		return vertexInputAttributeDescription;
	}

//
//IRenderer implementation
//

public:
	bool Init(Window* p_window) override;
	void Release() override;
	void Render() override;
};