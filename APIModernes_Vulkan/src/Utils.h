#pragma once

#include <cstdint>
#include <vector>

#include "Maths.h"

#include "vulkan/vulkan.h"

#pragma region App Parameters

#define APP_NAME	"API Graphiques modernes : vulkan"
#define WINDOW_WIDTH	720
#define WINDOW_HEIGHT	480

#define APP_VERSION_MAJOR	1
#define APP_VERSION_MINOR	0
#define APP_VERSION_PATCH	0

#define ENGINE_NAME "HM_Engine"

#define ENGINE_VERSION_MAJOR	1
#define ENGINE_VERSION_MINOR	0
#define ENGINE_VERSION_PATCH	0	

#pragma endregion App Parameters

struct Vertex
{
	Vector3 pos;
	Vector3 color;
};

//TODO : Add VKUtils namespace because why not kekew
#pragma region Utils Vulkan Renderer
struct DeviceSupportedQueues
{
	uint32_t graphicsFamily = UINT32_MAX;
	uint32_t presentFamily	= UINT32_MAX;
	
	bool isComplete()
	{
		return (graphicsFamily != UINT32_MAX) && (presentFamily != UINT32_MAX);
	}
};

struct SwapChainCapabilities
{
	VkSurfaceCapabilitiesKHR		surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR>	presentModes;

	bool isComplete()
	{
		return !formats.empty() && !presentModes.empty();
	}
};

struct PhysicalDeviceDescription
{
	VkPhysicalDevice			physicalDevice;
	VkPhysicalDeviceFeatures	deviceFeatures;

	DeviceSupportedQueues		supportedQueues;
	SwapChainCapabilities		swapChainParameters;
};

struct SwapChainDescription
{	
	VkSwapchainKHR				vkSwapChain;

	VkFormat					imageFormat;
	VkExtent2D					extent;

	std::vector<VkImage>		images;
	std::vector<VkImageView>	imageViews;

	std::vector<VkFramebuffer>	frameBuffers;
};

struct GraphicPipelineDescription
{
	std::vector<VkPipelineShaderStageCreateInfo> mShaderStages;

	VkPipelineLayout	vkPipelineLayout;
	VkRenderPass		vkRenderPass;
	VkPipeline			vkPipeline;

	const int MAX_CONCURENT_FRAMES = 2; //Would like this parametrable !
};
#pragma endregion Vulkan Renderer

std::vector<char> ParseShaderFile(const char* p_fileName);