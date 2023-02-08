#include <set>

#include "Utils.h"

#include "GLFW/glfw3.h"

#include "VKRenderer.h"


bool VKRenderer::CreateVKInstance()
{
	//
	// VkApplicationInfo initialisation
	//

	VkApplicationInfo applicationInfo{};

	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = APP_NAME;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
	applicationInfo.pEngineName = ENGINE_NAME;
	applicationInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	//
	// VkApplicationInfo initialisation
	//

	VkInstanceCreateInfo instanceCreateInfo{};

	uint32_t glfwExtentionCount = 0;
	const char** glfwExtentionsChr = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = glfwExtentionCount;
	instanceCreateInfo.ppEnabledExtensionNames = glfwExtentionsChr;
	instanceCreateInfo.enabledLayerCount = 0;

	//
	//Validation layers
	//

#ifdef _DEBUG
	if (this->CanEnableValidationLayers(this->mValidationLayers))
	{
		instanceCreateInfo.enabledLayerCount = (uint32_t)this->mValidationLayers.size();	//Conversion :/
		instanceCreateInfo.ppEnabledLayerNames = this->mValidationLayers.data();
	}
	else
	{
		return false;
	}
#endif // _DEBUG

	return vkCreateInstance(&instanceCreateInfo, nullptr, &this->mVKInstance) == VK_SUCCESS;
}

bool VKRenderer::CanEnableValidationLayers(const std::vector<const char*>& p_validationLayers)
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties>layerProperties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

	//Check if all layers are supported
	for (const char* layer : p_validationLayers)
	{
		bool isFound = false;
		for (const VkLayerProperties &layerProperty : layerProperties)
		{
			isFound = strcmp(layerProperty.layerName, layer);
			if (isFound)
				break;
		}

		if (!isFound)
			return false;
	}

	return true;
}

bool VKRenderer::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(this->mVKInstance, &deviceCount, nullptr);

	if (deviceCount == 0)
		return false;

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(this->mVKInstance, &deviceCount, devices.data());

	this->mPhysicalDevice = this->GetBestDevice(devices);

	return true;
}

bool VKRenderer::CheckDeviceExtentions(const VkPhysicalDevice& p_device)
{
	uint32_t extentionCount = 0;
	vkEnumerateDeviceExtensionProperties(p_device, nullptr, &extentionCount, nullptr);

	std::vector<VkExtensionProperties> deviceExtentions;
	vkEnumerateDeviceExtensionProperties(p_device, nullptr, &extentionCount, deviceExtentions.data());

	std::set<std::string> requiredExtentions(this->mExtensions.begin(), this->mExtensions.end());

	for (const VkExtensionProperties& extention : deviceExtentions)
	{
		requiredExtentions.erase(extention.extensionName);
	}

	return requiredExtentions.empty();
}

bool VKRenderer::DeviceIsSupported(const VkPhysicalDevice& p_device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(p_device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(p_device, &deviceFeatures);

	bool extentionsSupported = this->CheckDeviceExtentions(p_device);

	return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader);
}

DeviceSupportedQueues VKRenderer::GetDeviceSupportedQueues(const VkPhysicalDevice& p_device)
{
	DeviceSupportedQueues result;

	uint32_t queueCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(p_device, &queueCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(p_device, &queueCount, queueFamilies.data());

	uint32_t i = 0;
	for (const VkQueueFamilyProperties& queues : queueFamilies)
	{
		if (queues.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			result.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(p_device, i, this->mRenderingSurface, &presentSupport);

		if (presentSupport)
			result.presentFamily = i;

		if (result.isComplete())
		{
			return result;
		}

		i++;
	}

	return result;
}

SwapChainCapabilities VKRenderer::GetSwapChainParameters(const VkPhysicalDevice& p_device)
{
	SwapChainCapabilities swapChainParameters;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device, this->mRenderingSurface, &swapChainParameters.surfaceCapabilities);

	uint32_t formatCount = 0;

	vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, this->mRenderingSurface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		swapChainParameters.formats = std::vector<VkSurfaceFormatKHR>(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, this->mRenderingSurface, &formatCount, swapChainParameters.formats.data());
	}

	uint32_t modesCount = 0;

	vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, this->mRenderingSurface, &modesCount, nullptr);

	if (modesCount != 0)
	{
		swapChainParameters.presentModes = std::vector<VkPresentModeKHR>(modesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, this->mRenderingSurface, &modesCount, swapChainParameters.presentModes.data());
	}

	return swapChainParameters;
}

PhysicalDeviceDescription VKRenderer::GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices)
{
	PhysicalDeviceDescription result;

	bool deviceFound = false;

	for (const VkPhysicalDevice& device : p_devices)
	{
		if (!this->DeviceIsSupported(device))
			continue;
		
		DeviceSupportedQueues supportedQueues = this->GetDeviceSupportedQueues(device);

		if (!supportedQueues.isComplete())
			continue;
		
		SwapChainCapabilities parameters = this->GetSwapChainParameters(device);

		if (!parameters.isComplete())
		{
			continue; //Crash the program somehow
		}
		else
		{
			VkPhysicalDeviceFeatures deviceFeatures;
			vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

			result.physicalDevice = device;
			result.supportedQueues = supportedQueues;
			result.deviceFeatures = deviceFeatures;
			result.swapChainParameters = parameters;
			break;
		}
	}

	return result;
}

bool VKRenderer::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
	
	std::vector<int32_t> queuesIdx;
	
	queuesIdx.push_back(this->mPhysicalDevice.supportedQueues.presentFamily);
	queuesIdx.push_back(this->mPhysicalDevice.supportedQueues.graphicsFamily);
	
	constexpr float queuePriorities = 1.0f;

	for(int32_t queue : queuesIdx)
	{ 
		VkDeviceQueueCreateInfo deviceQueueCreateInfo{};

		deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		deviceQueueCreateInfo.queueFamilyIndex = queue;
		deviceQueueCreateInfo.queueCount = 1;
		deviceQueueCreateInfo.pQueuePriorities = &queuePriorities;

		deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo{};

	deviceCreateInfo.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pEnabledFeatures			= &this->mPhysicalDevice.deviceFeatures;
	deviceCreateInfo.pQueueCreateInfos			= deviceQueueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount		= (uint32_t)deviceQueueCreateInfos.size();
	deviceCreateInfo.enabledLayerCount			= 0;
	deviceCreateInfo.ppEnabledExtensionNames	= this->mExtensions.data();
	deviceCreateInfo.enabledExtensionCount		= (uint32_t)this->mExtensions.size();

#ifdef _DEBUG
	deviceCreateInfo.ppEnabledLayerNames = this->mValidationLayers.data();
	deviceCreateInfo.enabledLayerCount	 = (uint32_t)this->mValidationLayers.size();
#endif // _DEBUG

	return vkCreateDevice(this->mPhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &this->mLogicalDevice) == VK_SUCCESS;
}

//Inline really matter ? feel like compiler will do it anyway
inline VkSurfaceFormatKHR GetSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
{
	for (const VkSurfaceFormatKHR& format :formats)
	{
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return format;
		}
	}

	return formats[0];
}

inline VkExtent2D VKRenderer::GetSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX)
	{
		return surfaceCapabilities.currentExtent;
	}
	else
	{
		int width, height;

		glfwGetFramebufferSize(this->mRenderingWindow->mWindow, &width, &height);

		VkExtent2D currentExtent =
		{
			(uint32_t)(width),
			(uint32_t)(height)
		};

		return currentExtent;
	}
}

bool VKRenderer::CreateSwapChain()
{
	VkSurfaceFormatKHR	imageFormat = GetSwapchainSurfaceFormat(this->mPhysicalDevice.swapChainParameters.formats);
	VkExtent2D			imageExtent = GetSwapchainExtent(this->mPhysicalDevice.swapChainParameters.surfaceCapabilities);

	uint32_t imageCount = this->mPhysicalDevice.swapChainParameters.surfaceCapabilities.minImageCount + 1;

	if (this->mPhysicalDevice.swapChainParameters.surfaceCapabilities.maxImageCount > 0 && imageCount > this->mPhysicalDevice.swapChainParameters.surfaceCapabilities.maxImageCount)
		imageCount = this->mPhysicalDevice.swapChainParameters.surfaceCapabilities.maxImageCount;


	//
	//Fill the structure
	//

	VkSwapchainCreateInfoKHR swapchainCreateInfoKHR{};

	swapchainCreateInfoKHR.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfoKHR.surface			= this->mRenderingSurface;
	swapchainCreateInfoKHR.oldSwapchain		= VK_NULL_HANDLE;
	swapchainCreateInfoKHR.presentMode		= VK_PRESENT_MODE_FIFO_KHR;		//Hardcoded
	swapchainCreateInfoKHR.clipped			= VK_TRUE;

	swapchainCreateInfoKHR.imageFormat		= imageFormat.format;
	swapchainCreateInfoKHR.imageColorSpace	= imageFormat.colorSpace;

	swapchainCreateInfoKHR.imageExtent		= imageExtent;

	swapchainCreateInfoKHR.imageArrayLayers = 1;
	swapchainCreateInfoKHR.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	swapchainCreateInfoKHR.minImageCount	= imageCount;
	
	if (this->mPhysicalDevice.supportedQueues.graphicsFamily != this->mPhysicalDevice.supportedQueues.presentFamily)
	{
		uint32_t queueFamilyIndices[] = { this->mPhysicalDevice.supportedQueues.graphicsFamily, this->mPhysicalDevice.supportedQueues.presentFamily };

		swapchainCreateInfoKHR.imageSharingMode			= VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfoKHR.queueFamilyIndexCount	= 2;
		swapchainCreateInfoKHR.pQueueFamilyIndices		= queueFamilyIndices;
	}
	else 
	{
		swapchainCreateInfoKHR.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
	}

	swapchainCreateInfoKHR.preTransform = this->mPhysicalDevice.swapChainParameters.surfaceCapabilities.currentTransform;
	swapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	//
	//Acutally create and fill the swapchain struct
	//

	SwapChainDescription createdSwapChain {};

	if (vkCreateSwapchainKHR(this->mLogicalDevice, &swapchainCreateInfoKHR, nullptr, &createdSwapChain.vkSwapChain) == VK_SUCCESS)
	{
		uint32_t imageCount = 0;
		
		vkGetSwapchainImagesKHR(this->mLogicalDevice, createdSwapChain.vkSwapChain, &imageCount, nullptr);
		
		createdSwapChain.images		= std::vector<VkImage>(imageCount);
		createdSwapChain.imageViews = std::vector<VkImageView>(imageCount);
		
		vkGetSwapchainImagesKHR(this->mLogicalDevice, createdSwapChain.vkSwapChain, &imageCount, createdSwapChain.images.data());

		createdSwapChain.extent = imageExtent;
		createdSwapChain.imageFormat = imageFormat.format;

		int i = 0;
		for (VkImageView& imageView : this->mSwapChain.imageViews)
		{
			VkImageViewCreateInfo imageViewCreateInfo{};

			imageViewCreateInfo.sType		= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCreateInfo.image		= createdSwapChain.images[i];
			imageViewCreateInfo.format		= imageFormat.format;
			imageViewCreateInfo.viewType	= VK_IMAGE_VIEW_TYPE_2D;

			imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
			imageViewCreateInfo.subresourceRange.levelCount = 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount = 1;

			vkCreateImageView(this->mLogicalDevice, &imageViewCreateInfo, nullptr, &imageView);

			i++;
		}

		this->mSwapChain = createdSwapChain;
		return true;
	}
	else
	{
		return false;
	}
}

//
//IRenderer implementation
//

bool VKRenderer::Init(Window* p_window)
{
	__super::Init(p_window);

	bool result = this->CreateVKInstance();
	
	result &= glfwCreateWindowSurface(this->mVKInstance, this->mRenderingWindow->mWindow , nullptr, &this->mRenderingSurface) == VK_SUCCESS;

	result &= this->PickPhysicalDevice();
	result &= this->CreateLogicalDevice();
	result &= this->CreateSwapChain();
	
	vkGetDeviceQueue(mLogicalDevice, this->mPhysicalDevice.supportedQueues.presentFamily, 0, &mPresentQueue); //Does not return VK_RESULT //Move this to a struct ?

	return result;
}

void VKRenderer::Release()
{
	for (const VkImageView& imageView : this->mSwapChain.imageViews)
		vkDestroyImageView(this->mLogicalDevice, imageView, nullptr);

	vkDestroySwapchainKHR(this->mLogicalDevice, this->mSwapChain.vkSwapChain, nullptr);

	vkDestroySurfaceKHR(this->mVKInstance, this->mRenderingSurface, nullptr);

	vkDestroyDevice(this->mLogicalDevice, nullptr);

	//
	//Destory the instance at the very end !
	//

	vkDestroyInstance(this->mVKInstance, nullptr);
}
