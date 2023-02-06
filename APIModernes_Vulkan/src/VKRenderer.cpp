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

//TODO: Better way to select gpu ! VKRenderer::GetBestDevice
//TODO: Refactor VKRenderer::GetBestDevice
//TODO : VKRenderer::GetBestDevice Crash the program if no suiatable gpu
PhysicalDeviceDescription VKRenderer::GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices)
{
	PhysicalDeviceDescription result;

	bool deviceFound = false;

	for (const VkPhysicalDevice& device : p_devices)
	{
		if (deviceFound)
			break;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader)
		{
			DeviceSupportedQueues supportedQueues;

			uint32_t queueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilies(queueCount);
			vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queueFamilies.data());
			
			uint32_t i = 0;
			for(const VkQueueFamilyProperties &queues : queueFamilies)
			{
				if(queues.queueFlags & VK_QUEUE_GRAPHICS_BIT)
					supportedQueues.graphicsFamily = i;
		
				VkBool32 presentSupport = false;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->mRenderingSurface, &presentSupport);

				if (presentSupport)
					supportedQueues.presentFamily = i;

				if (supportedQueues.isComplete())
				{
					deviceFound = true;

					result.physicalDevice	= device;
					result.supportedQueues	= supportedQueues;
					result.deviceFeatures	= deviceFeatures;
					break;
				}
				
				i++;
			}
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
	
	const float queuePriorities = 1.0f;

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

	deviceCreateInfo.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pEnabledFeatures		= &this->mPhysicalDevice.deviceFeatures;
	deviceCreateInfo.pQueueCreateInfos		= deviceQueueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount	= deviceQueueCreateInfos.size();
	deviceCreateInfo.enabledLayerCount		= 0;

#ifdef _DEBUG
	deviceCreateInfo.enabledLayerCount	 = this->mValidationLayers.size();
	deviceCreateInfo.ppEnabledLayerNames = this->mValidationLayers.data();
#endif // _DEBUG

	return vkCreateDevice(this->mPhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &this->mLogicalDevice) == VK_SUCCESS;
}

bool VKRenderer::Init(const Window* p_window)
{
	bool result = this->CreateVKInstance();
	
	result &= glfwCreateWindowSurface(this->mVKInstance, p_window->mWindow , nullptr, &this->mRenderingSurface) == VK_SUCCESS;
	result &= PickPhysicalDevice();
	result &= CreateLogicalDevice();
	
	vkGetDeviceQueue(mLogicalDevice, this->mPhysicalDevice.supportedQueues.presentFamily, 0, &mPresentQueue); //Does not return VK_RESULT ??

	return result;
}

void VKRenderer::Release()
{
	vkDestroyDevice(this->mLogicalDevice, nullptr);
	vkDestroySurfaceKHR(this->mVKInstance, this->mRenderingSurface, nullptr);

	//Destory the instance at the very end !
	vkDestroyInstance(this->mVKInstance, nullptr);
}
