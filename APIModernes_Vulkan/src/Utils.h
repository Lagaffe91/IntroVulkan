#pragma once

#include <cstdint>
#include <vector>

#include "glm/glm.hpp"
#include <glm/gtx/hash.hpp>

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

#define MODEL_PATH "models/model.obj"

#pragma endregion App Parameters

struct Vertex
{	
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 textCoords;

	bool operator==(const Vertex& other) const {
		return pos == other.pos && color == other.color && textCoords == other.textCoords;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.textCoords) << 1);
		}
	};
}



struct UniformBufferObject 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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
	VkPhysicalDeviceProperties  deviceProperties;
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

struct DepthRessources
{
	VkImage			depthImage;
	VkDeviceMemory	depthMemory;
	VkImageView		depthImageView;
};
#pragma endregion Vulkan Renderer

std::vector<char> ParseShaderFile(const char* p_fileName);