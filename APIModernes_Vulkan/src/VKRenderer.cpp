#include "Utils.h"

#include "GLFW/glfw3.h"

#include "VKRenderer.h"
bool VKRenderer::CreateVKInstance()
{
	//
	// VkApplicationInfo initialisation
	//

	VkApplicationInfo applicationInfo;

	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName = APP_NAME;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
	applicationInfo.pEngineName = ENGINE_NAME;
	applicationInfo.engineVersion = VK_MAKE_VERSION(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	//
	// VkApplicationInfo initialisation
	//

	VkInstanceCreateInfo instanceCreateInfo;

	uint32_t glfwExtentionCount = 0;
	const char** glfwExtentionsChr = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = glfwExtentionCount;
	instanceCreateInfo.ppEnabledLayerNames = glfwExtentionsChr;
	instanceCreateInfo.enabledLayerCount = 0;

#ifdef _DEBUG
	uint32_t layerCount = 0;
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" }; //More ?

	if (this->EnableValidationLayers(validationLayers, &layerCount))
	{
		instanceCreateInfo.enabledLayerCount = layerCount;
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		return false;
	}
#endif // _DEBUG


	VkResult creationResult = vkCreateInstance(&instanceCreateInfo, nullptr, &this->mVKInstance);
	return creationResult == VK_SUCCESS;
}

bool VKRenderer::EnableValidationLayers(const std::vector<const char*>& validationLayers, uint32_t* layerCount)
{
	vkEnumerateInstanceLayerProperties(layerCount, nullptr);

	std::vector<VkLayerProperties>layerProperties(*layerCount);
	vkEnumerateInstanceLayerProperties(layerCount, layerProperties.data());

	//Check if all layers are supported
	for (const char* layer : validationLayers)
	{
		bool isFound = false;
		for (VkLayerProperties layerProperty : layerProperties)
		{

		}
	}

	VkResult result = VK_SUCCESS;
	return result == VK_SUCCESS;


}

bool VKRenderer::Init()
{
	bool result = this->CreateVKInstance();
	
	//result &= 

	return result;
}

void VKRenderer::Release()
{
	vkDestroyInstance(this->mVKInstance, nullptr);
}
