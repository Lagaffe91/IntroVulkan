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


	//Validation layers
#ifdef _DEBUG
	
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" }; //More ?

	if (this->CanEnableValidationLayers(validationLayers))
	{
		instanceCreateInfo.enabledLayerCount = (uint32_t)validationLayers.size(); //Conversion :/
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
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

//TODO: Better way to select gpu !
PhysicalDeviceDescription VKRenderer::GetBestDevice(const std::vector<VkPhysicalDevice>& p_devices)
{
	PhysicalDeviceDescription result;

	for (const VkPhysicalDevice& device : p_devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
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
					supportedQueues.graphicalQueue = i;

				if (supportedQueues.isComplete())
				{
					result.physicalDevice = device;
					result.supportedQueues = supportedQueues;
					break;
				}
				
				i++;
			}
		}
	}

	//TODO : Should crash the program here (no suiatable gpu)
	return result;
}

bool VKRenderer::CreateLogicalDevice()
{
	VkDeviceCreateInfo deviceCreateInfo{};

	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	return vkCreateDevice(this->mPhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &this->mLogicalDevice) == VK_SUCCESS;
}

bool VKRenderer::Init()
{
	bool result = this->CreateVKInstance();
	
	result &= PickPhysicalDevice();
	//result &= 

	return result;
}

void VKRenderer::Release()
{
	vkDestroyInstance(this->mVKInstance, nullptr);
	vkDestroyDevice(this->mLogicalDevice, nullptr);
}
