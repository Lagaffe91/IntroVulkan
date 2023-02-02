#pragma once

#include <cstdint>

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

struct PhysicalDeviceDescription
{
	VkPhysicalDevice			physicalDevice;
	VkPhysicalDeviceFeatures	deviceFeatures;

	DeviceSupportedQueues		supportedQueues;
};
#pragma endregion Vulkan Renderer