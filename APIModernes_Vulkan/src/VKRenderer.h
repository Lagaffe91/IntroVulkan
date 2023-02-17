#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <array>

#include "Utils.h"

#include "IRenderer.h"

class VKRenderer : public IRenderer
{
private :
	const std::vector<Vertex> vertices = 
	{
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f} , {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f}  , {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f} , {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};

	const std::vector<uint16_t> indices = {0, 1, 2, 2, 3, 0	};

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
	
	//-------
	std::vector<VkDescriptorSet>	mDescriptorSets;
	VkDescriptorSetLayout			mDescriptorSetLayout;
	VkDescriptorPool				mDescriptorPool;
	//-------

	//------
	VkCommandPool	mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffer;
	//------

	//------ Sync objects
	std::vector<VkSemaphore>	mRenderingSemaphore;
	std::vector<VkSemaphore>	mImageAviableSemaphore;
	std::vector<VkFence>		mPresentFence;
	//------

	//------ TODO : this would fit in a Model class
	VkBuffer		mVertexBuffer;
	VkDeviceMemory	mVertexBufferMemory;
	VkBuffer		mIndexBuffer;
	VkDeviceMemory	mIndexBufferMemory;
	VkImage			mTextureImage;
	VkDeviceMemory	mTextureImageMemory;
	VkImageView		mTextureImageView;
	VkSampler		mTextureSampler;
	//


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

	bool CreateDescriptorSetLayout();

	bool CreateDescriptorPool();

	bool CreateDescriptorSets();

	bool SetupGraphicsPipeline();

	bool CreateFrameBuffers();

	bool CreateCommandBuffer();

	bool CreateTextureImage();

	bool CreateTextureImageView(); 

	bool CreateTextureSampler();

	bool CreateVertexBuffer();

	bool CreateIndexBuffer();

	void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex);

	bool CreateSyncObjects();

	uint32_t FindMemoryType(const uint32_t& p_filterBits, VkMemoryPropertyFlags properties);

	bool CreateBuffer(VkDeviceSize p_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkBuffer& p_buffer, VkDeviceMemory& p_bufferMemory);
	bool CreateImage(uint32_t p_width, uint32_t p_height, VkFormat p_format, VkImageTiling p_tiling, VkImageUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkImage& p_image, VkDeviceMemory& p_imageMemory);

	void CopyBufferToImage(VkBuffer p_buffer, VkImage p_image, uint32_t p_width, uint32_t p_height);
	void TransitionImageLayout(VkImage p_image, VkFormat p_format, VkImageLayout p_oldLayout, VkImageLayout p_newLayout);
	void CopyBuffer(VkBuffer p_srcBuffer, VkBuffer p_dstBuffer, VkDeviceSize p_size);

	VkImageView CreateImageView(VkImage p_image, VkFormat p_format);

	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer p_commandBuffer);

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

	static std::array<VkVertexInputAttributeDescription, 3> GetAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 3> vertexInputAttributeDescription{};

		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[0].offset = offsetof(Vertex, Vertex::pos);

		vertexInputAttributeDescription[1].binding = 0;
		vertexInputAttributeDescription[1].location = 1;
		vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[1].offset = offsetof(Vertex, Vertex::color);

		vertexInputAttributeDescription[2].binding = 0;
		vertexInputAttributeDescription[2].location = 2;
		vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescription[2].offset = offsetof(Vertex, Vertex::textCoords);

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