#include <set>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "glm/gtc/matrix_transform.hpp"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "Utils.h"

#include "VKRenderer.h"


bool VKRenderer::CreateVKInstance()
{
	//
	// VkApplicationInfo initialisation
	//

	VkApplicationInfo applicationInfo{};

	applicationInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pApplicationName	= APP_NAME;
	applicationInfo.applicationVersion	= VK_MAKE_VERSION(APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_PATCH);
	applicationInfo.pEngineName			= ENGINE_NAME;
	applicationInfo.engineVersion		= VK_MAKE_VERSION(ENGINE_VERSION_MAJOR, ENGINE_VERSION_MINOR, ENGINE_VERSION_PATCH);
	applicationInfo.apiVersion			= VK_API_VERSION_1_3;

	//
	// VkApplicationInfo initialisation
	//

	VkInstanceCreateInfo instanceCreateInfo{};

	uint32_t glfwExtentionCount = 0;
	const char** glfwExtentionsChr = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);

	instanceCreateInfo.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo			= &applicationInfo;
	instanceCreateInfo.enabledExtensionCount	= glfwExtentionCount;
	instanceCreateInfo.ppEnabledExtensionNames	= glfwExtentionsChr;
	instanceCreateInfo.enabledLayerCount		= 0;

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
			continue;
		
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceFeatures.samplerAnisotropy == VK_FALSE)
			continue;
		
		//Device found
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		result.physicalDevice = device;
		result.supportedQueues = supportedQueues;
		result.deviceFeatures = deviceFeatures;
		result.swapChainParameters = parameters;
		result.deviceProperties = physicalDeviceProperties;
		break;
	}

	return result;
}

bool VKRenderer::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
	
	std::set<uint32_t> queuesIdx = {
		this->mPhysicalDevice.supportedQueues.presentFamily,
		this->mPhysicalDevice.supportedQueues.graphicsFamily
	};

	constexpr float queuePriorities = 1.0f;

	for (uint32_t queue : queuesIdx)
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

	bool result = vkCreateDevice(this->mPhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &this->mLogicalDevice) == VK_SUCCESS;
		
	vkGetDeviceQueue(this->mLogicalDevice, this->mPhysicalDevice.supportedQueues.graphicsFamily, 0, &this->mGraphicsQueue);
	vkGetDeviceQueue(this->mLogicalDevice, this->mPhysicalDevice.supportedQueues.presentFamily , 0, &this->mPresentQueue);

	return result;
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
		
		createdSwapChain.images.resize(imageCount);
		createdSwapChain.imageViews.resize(imageCount);
		
		vkGetSwapchainImagesKHR(this->mLogicalDevice, createdSwapChain.vkSwapChain, &imageCount, createdSwapChain.images.data());

		createdSwapChain.extent = imageExtent;
		createdSwapChain.imageFormat = imageFormat.format;

		for (int i = 0; i < imageCount; i++)
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
			imageViewCreateInfo.subresourceRange.baseMipLevel	= 0;
			imageViewCreateInfo.subresourceRange.levelCount		= 1;
			imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
			imageViewCreateInfo.subresourceRange.layerCount		= 1;

			vkCreateImageView(this->mLogicalDevice, &imageViewCreateInfo, nullptr, &createdSwapChain.imageViews[i]);
		}

		this->mSwapChain = createdSwapChain;
		return true;
	}
	else
	{
		return false;
	}
}

bool VKRenderer::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};

	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};

	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};

	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	descriptorSetLayoutCreateInfo.pBindings = bindings.data();

	return vkCreateDescriptorSetLayout(this->mLogicalDevice, &descriptorSetLayoutCreateInfo, nullptr, &this->mDescriptorSetLayout) == VK_SUCCESS;
}

bool VKRenderer::CreateUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	mUniformBuffers.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);
	mUniformBuffersMemory.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);
	mUniformBuffersMap.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);
	
	bool result = true;

	for (size_t i = 0; i < this->mGraphicsPipeline.MAX_CONCURENT_FRAMES; i++)
	{
		result &= this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->mUniformBuffers[i], this->mUniformBuffersMemory[i]);

		vkMapMemory(this->mLogicalDevice, this->mUniformBuffersMemory[i], 0, bufferSize, 0, &this->mUniformBuffersMap[i]);
	}
	return result;
}

bool VKRenderer::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> descriptorPoolSize{};

	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[0].descriptorCount = (uint32_t)(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);

	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize[1].descriptorCount = (uint32_t)(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);


	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};

	descriptorPoolCreateInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount	= (uint32_t)descriptorPoolSize.size();
	descriptorPoolCreateInfo.pPoolSizes		= descriptorPoolSize.data();
	descriptorPoolCreateInfo.maxSets		= (uint32_t)(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);

	return vkCreateDescriptorPool(this->mLogicalDevice, &descriptorPoolCreateInfo, nullptr, &this->mDescriptorPool) == VK_SUCCESS;
}

bool VKRenderer::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES, this->mDescriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};

	descriptorSetAllocateInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool		= this->mDescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount	= (uint32_t)(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);
	descriptorSetAllocateInfo.pSetLayouts			= layouts.data();

	this->mDescriptorSets.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);

	bool result = vkAllocateDescriptorSets(this->mLogicalDevice, &descriptorSetAllocateInfo, this->mDescriptorSets.data()) == VK_SUCCESS;

	for (size_t i = 0; i < this->mGraphicsPipeline.MAX_CONCURENT_FRAMES; i++)
	{
		VkDescriptorBufferInfo descriptorUniformBufferInfo{};

		descriptorUniformBufferInfo.buffer	= this->mUniformBuffers[i];
		descriptorUniformBufferInfo.offset	= 0;
		descriptorUniformBufferInfo.range	= sizeof(UniformBufferObject);

		VkDescriptorImageInfo descriptorImageInfo{};

		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.imageView	= this->mTextureImageView;
		descriptorImageInfo.sampler		= this->mTextureSampler;

		std::array<VkWriteDescriptorSet, 2>  writeDescriptorSet{}; //This will need to be an array when i'll do maths stuff

		writeDescriptorSet[0].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[0].dstSet			= this->mDescriptorSets[i];
		writeDescriptorSet[0].dstBinding		= 0;
		writeDescriptorSet[0].dstArrayElement	= 0;
		writeDescriptorSet[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet[0].descriptorCount	= 1;
		writeDescriptorSet[0].pBufferInfo		= &descriptorUniformBufferInfo;
		writeDescriptorSet[0].pImageInfo		= nullptr;
		writeDescriptorSet[0].pTexelBufferView	= nullptr;

		writeDescriptorSet[1].sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet[1].dstSet			= this->mDescriptorSets[i];
		writeDescriptorSet[1].dstBinding		= 1;
		writeDescriptorSet[1].dstArrayElement	= 0;
		writeDescriptorSet[1].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSet[1].descriptorCount	= 1;
		writeDescriptorSet[1].pImageInfo		= &descriptorImageInfo;

		vkUpdateDescriptorSets(this->mLogicalDevice, writeDescriptorSet.size(), writeDescriptorSet.data(), 0, nullptr);
		
 	}

	return result;
}

bool VKRenderer::SetupGraphicsPipeline()
{
	//
	//Fixed functions
	//

	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};

	pipelineDynamicStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pipelineDynamicStateCreateInfo.dynamicStateCount	= static_cast<uint32_t>(dynamicStates.size());
	pipelineDynamicStateCreateInfo.pDynamicStates		= dynamicStates.data();

	auto bindingDescription = VKRenderer::GetBindingDescription();
	auto attributeDescriptions = VKRenderer::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};

	pipelineVertexInputStateCreateInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount	= 1;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions		= &bindingDescription;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions		= attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};

	pipelineInputAssemblyStateCreateInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology				= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;


	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};

	pipelineViewportStateCreateInfo.sType			= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineViewportStateCreateInfo.viewportCount	= 1;
	pipelineViewportStateCreateInfo.scissorCount	= 1;


	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};

	pipelineRasterizationStateCreateInfo.sType						= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineRasterizationStateCreateInfo.depthClampEnable			= VK_FALSE;
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable	= VK_FALSE;
	pipelineRasterizationStateCreateInfo.polygonMode				= VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.lineWidth					= 1.0f;
	pipelineRasterizationStateCreateInfo.cullMode					= VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthBiasEnable			= VK_FALSE;


	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};

	pipelineMultisampleStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineMultisampleStateCreateInfo.sampleShadingEnable	= VK_FALSE;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;



	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};

	pipelineColorBlendAttachmentState.colorWriteMask	= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.blendEnable		= VK_FALSE;


	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
	
	pipelineColorBlendStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineColorBlendStateCreateInfo.logicOpEnable		= VK_FALSE;
	pipelineColorBlendStateCreateInfo.attachmentCount	= 1;
	pipelineColorBlendStateCreateInfo.pAttachments		= &pipelineColorBlendAttachmentState;

	//
	//Shaders
	//

	std::vector<char> vertexByteCode	= ParseShaderFile("./shaders/triangle.vert.spv");
	std::vector<char> fragmentByteCode	= ParseShaderFile("./shaders/triangle.frag.spv");

	if(vertexByteCode.empty() || fragmentByteCode.empty())
		return false;

	this->mVertexShader		= this->LoadShader(vertexByteCode);
	this->mFragmentShader	= this->LoadShader(fragmentByteCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};

	vertShaderStageInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage	= VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module	= this->mVertexShader;
	vertShaderStageInfo.pName	= "main";


	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};

	fragShaderStageInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage	= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module	= this->mFragmentShader;
	fragShaderStageInfo.pName	= "main";
		
	this->mGraphicsPipeline.mShaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	//
	//Pipeline layout
	//

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &this->mDescriptorSetLayout;

	if (vkCreatePipelineLayout(this->mLogicalDevice, &pipelineLayoutInfo, nullptr, &this->mGraphicsPipeline.vkPipelineLayout) != VK_SUCCESS)
		return false;

	//
	//Render pass
	//

	VkAttachmentReference subPassColorAttachement{};

	subPassColorAttachement.attachment = 0;
	subPassColorAttachement.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachmentDescription{};

	depthAttachmentDescription.format			= this->FindDepthFormat();
	depthAttachmentDescription.samples			= VK_SAMPLE_COUNT_1_BIT;
	depthAttachmentDescription.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentDescription.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachmentDescription.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachmentDescription.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpassDescription{};
	
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pipelineBindPoint	= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments	= &subPassColorAttachement;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;


	VkAttachmentDescription colorAttachment{};
	
	colorAttachment.format			= this->mSwapChain.imageFormat;
	colorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkSubpassDependency subpassDependency{};

	subpassDependency.srcSubpass	= VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass	= 0;
	subpassDependency.srcStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstStageMask	= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


	std::array<VkAttachmentDescription, 2> attachmentDescription{colorAttachment, depthAttachmentDescription};


	VkRenderPassCreateInfo renderPassCreateInfo{};

	renderPassCreateInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount	= attachmentDescription.size();
	renderPassCreateInfo.pAttachments		= attachmentDescription.data();
	renderPassCreateInfo.subpassCount		= 1;
	renderPassCreateInfo.pSubpasses			= &subpassDescription;
	renderPassCreateInfo.dependencyCount	= 1;
	renderPassCreateInfo.pDependencies		= &subpassDependency;


	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};

	pipelineDepthStencilStateCreateInfo.sType				= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineDepthStencilStateCreateInfo.depthTestEnable		= VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable	= VK_TRUE;
	pipelineDepthStencilStateCreateInfo.depthCompareOp		= VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;



	if (vkCreateRenderPass(this->mLogicalDevice, &renderPassCreateInfo, nullptr, &this->mGraphicsPipeline.vkRenderPass) != VK_SUCCESS)
		return false;

	//
	//Graphics pipeline object
	//

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};

	graphicsPipelineCreateInfo.sType		= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.layout		= this->mGraphicsPipeline.vkPipelineLayout;
	graphicsPipelineCreateInfo.renderPass	= this->mGraphicsPipeline.vkRenderPass;
	graphicsPipelineCreateInfo.subpass		= 0;
	graphicsPipelineCreateInfo.stageCount	= 2;
	graphicsPipelineCreateInfo.pStages		= this->mGraphicsPipeline.mShaderStages.data();

	graphicsPipelineCreateInfo.pDynamicState		= &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.pVertexInputState	= &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState		= &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState	= &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState		= &pipelineColorBlendStateCreateInfo;

	graphicsPipelineCreateInfo.pDepthStencilState	= &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.basePipelineHandle	= VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex	= -1;

	return vkCreateGraphicsPipelines(this->mLogicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &this->mGraphicsPipeline.vkPipeline) == VK_SUCCESS;
}

VkFormat VKRenderer::FindSupportedFormat(const std::vector<VkFormat>& p_candidates, VkImageTiling p_tiling, VkFormatFeatureFlags p_features) {
	for (VkFormat format : p_candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(this->mPhysicalDevice.physicalDevice, format, &props);

		if (p_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & p_features) == p_features)
		{
			return format;
		}
		else if (p_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & p_features) == p_features)
		{
			return format;
		}
	}
}


VkFormat VKRenderer::FindDepthFormat()
{
	return this->FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT }, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool VKRenderer::CreateDepthRessources()
{
	bool result = false;

	VkFormat depthFormat = this->FindDepthFormat();

	result = this->CreateImage(this->mSwapChain.extent.width, this->mSwapChain.extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->mDepthRessources.depthImage, this->mDepthRessources.depthMemory);
	this->mDepthRessources.depthImageView = this->CreateImageView(this->mDepthRessources.depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	return result;
}

bool VKRenderer::CreateFrameBuffers()
{
	this->mSwapChain.frameBuffers.resize(this->mSwapChain.imageViews.size());

	for (int i = 0; i < this->mSwapChain.frameBuffers.capacity(); i++)
	{
		std::array<VkImageView,2> attachments = {
			this->mSwapChain.imageViews[i],
			this->mDepthRessources.depthImageView
		};

		VkFramebufferCreateInfo framebufferCreateInfo{};

		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.height = this->mSwapChain.extent.height;
		framebufferCreateInfo.width = this->mSwapChain.extent.width;
		framebufferCreateInfo.renderPass = this->mGraphicsPipeline.vkRenderPass;
		framebufferCreateInfo.layers = 1;
		framebufferCreateInfo.attachmentCount = attachments.size();
		framebufferCreateInfo.pAttachments = attachments.data();

		vkCreateFramebuffer(this->mLogicalDevice, &framebufferCreateInfo, nullptr, &this->mSwapChain.frameBuffers[i]) == VK_SUCCESS;
	}

	return true;
}

bool VKRenderer::CreateCommandBuffer()
{
	//
	//Command pool 
	//

	VkCommandPoolCreateInfo commandPoolCreateInfo{};

	commandPoolCreateInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex	= this->mPhysicalDevice.supportedQueues.graphicsFamily;

	bool result = vkCreateCommandPool(this->mLogicalDevice, &commandPoolCreateInfo, nullptr, &this->mCommandPool) == VK_SUCCESS;

	//
	//Command buffers
	//

	mCommandBuffer.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};

	commandBufferAllocateInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool			= this->mCommandPool;
	commandBufferAllocateInfo.commandBufferCount	= mCommandBuffer.size();
	commandBufferAllocateInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	result &= vkAllocateCommandBuffers(this->mLogicalDevice, &commandBufferAllocateInfo, this->mCommandBuffer.data()) == VK_SUCCESS;

	return result;
}

VkCommandBuffer VKRenderer::BeginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->mCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->mLogicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VKRenderer::EndSingleTimeCommands(VkCommandBuffer p_commandBuffer) {
	vkEndCommandBuffer(p_commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &p_commandBuffer;

	vkQueueSubmit(this->mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->mGraphicsQueue);

	vkFreeCommandBuffers(this->mLogicalDevice, this->mCommandPool, 1, &p_commandBuffer);
}
void VKRenderer::CopyBufferToImage(VkBuffer p_buffer, VkImage p_image, uint32_t p_width, uint32_t p_height) {
	VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands();

	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageOffset = { 0, 0, 0 };
	bufferImageCopy.imageExtent = {
		p_width,
		p_height,
		1
	};

	vkCmdCopyBufferToImage(commandBuffer, p_buffer, p_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);

	this->EndSingleTimeCommands(commandBuffer);
}

bool VKRenderer::CreateTextureImage()
{
	const char* filePath = "textures/texture.png"; //WOAH

	bool result = false;

	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(filePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		return false;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	result &= this->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(this->mLogicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->mLogicalDevice, stagingBufferMemory);

	stbi_image_free(pixels);

	result &= this->CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->mTextureImage, this->mTextureImageMemory);

	this->TransitionImageLayout(this->mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	this->CopyBufferToImage(stagingBuffer, this->mTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	this->TransitionImageLayout(this->mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(this->mLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->mLogicalDevice, stagingBufferMemory, nullptr);

	return result;
}

VkImageView VKRenderer::CreateImageView(VkImage p_image, VkFormat p_format, VkImageAspectFlags p_aspectFlags)
{
	VkImageViewCreateInfo imageViewCreateInfo{};

	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = p_image;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = p_format;
	imageViewCreateInfo.subresourceRange.aspectMask = p_aspectFlags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;

	vkCreateImageView(this->mLogicalDevice, &imageViewCreateInfo, nullptr, &imageView);

	return imageView;
}

bool VKRenderer::CreateTextureImageView()
{
	this->mTextureImageView = this->CreateImageView(this->mTextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

	return true;
}

void VKRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = this->BeginSingleTimeCommands();

	VkImageMemoryBarrier imageMemoryBarrier{};

	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.oldLayout = oldLayout;
	imageMemoryBarrier.newLayout = newLayout;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		return;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier
	);

	this->EndSingleTimeCommands(commandBuffer);
}

bool VKRenderer::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerCreateInfo{};

	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = this->mPhysicalDevice.deviceProperties.limits.maxSamplerAnisotropy; //MAXIMUM PODER
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	return vkCreateSampler(this->mLogicalDevice, &samplerCreateInfo, nullptr, &this->mTextureSampler) == VK_SUCCESS;
}


uint32_t VKRenderer::FindMemoryType(const uint32_t& p_filterBits, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

	vkGetPhysicalDeviceMemoryProperties(this->mPhysicalDevice.physicalDevice, &physicalDeviceMemoryProperties);

	for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) 
	{
		if ((p_filterBits & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;
	}
}

bool VKRenderer::CreateVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(this->vertices[0]) * this->vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	bool result = this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory) == VK_SUCCESS;

	//
	//Copy data to buffer
	//

	void* data;
	vkMapMemory(this->mLogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, this->vertices.data(), bufferSize);
	vkUnmapMemory(this->mLogicalDevice, stagingBufferMemory);

	//
	//stagering to vertex buffer
	//

	result&= this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->mVertexBuffer, this->mVertexBufferMemory) == VK_SUCCESS;

	this->CopyBuffer(stagingBuffer, this->mVertexBuffer, bufferSize);

	vkDestroyBuffer(this->mLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->mLogicalDevice, stagingBufferMemory, nullptr);

	return result;
}

void VKRenderer::CopyBuffer(VkBuffer p_srcBuffer, VkBuffer p_dstBuffer, VkDeviceSize p_size)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{};

	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandPool = this->mCommandPool;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(this->mLogicalDevice, &commandBufferAllocateInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};

	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy bufferCopy{};

	bufferCopy.size = p_size;

	vkCmdCopyBuffer(commandBuffer, p_srcBuffer, p_dstBuffer, 1, &bufferCopy);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(this->mGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->mGraphicsQueue);

	vkFreeCommandBuffers(this->mLogicalDevice, this->mCommandPool, 1, &commandBuffer);
}


void VKRenderer::RecordCommandBuffer(VkCommandBuffer& p_commandBuffer, uint32_t p_imageIndex)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo{};

	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(p_commandBuffer, &commandBufferBeginInfo);

	VkRenderPassBeginInfo renderPassBeginInfo{};

	renderPassBeginInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass			= this->mGraphicsPipeline.vkRenderPass;
	renderPassBeginInfo.framebuffer			= this->mSwapChain.frameBuffers[p_imageIndex];
	renderPassBeginInfo.renderArea.extent	= this->mSwapChain.extent;
	renderPassBeginInfo.renderArea.offset	= { 0,0 };

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
	clearValues[1].depthStencil = { 1.0f, 0 };

	renderPassBeginInfo.clearValueCount		= clearValues.size();
	renderPassBeginInfo.pClearValues		= clearValues.data();

	vkCmdBeginRenderPass(p_commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(p_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mGraphicsPipeline.vkPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width	= (float)(this->mSwapChain.extent.width);
	viewport.height = (float)(this->mSwapChain.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vkCmdSetViewport(p_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = this->mSwapChain.extent;

	vkCmdSetScissor(p_commandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(p_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mGraphicsPipeline.vkPipeline);

	vkCmdBindDescriptorSets(p_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->mGraphicsPipeline.vkPipelineLayout, 0, 1, &this->mDescriptorSets[this->mCurrentFrame], 0, nullptr);

	VkBuffer vertexBuffers[] = { this->mVertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(p_commandBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(p_commandBuffer, this->mIndexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdDrawIndexed(p_commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	vkCmdEndRenderPass(p_commandBuffer);
	vkEndCommandBuffer(p_commandBuffer);
}

VkShaderModule VKRenderer::LoadShader(const std::vector<char>& p_byteCode)
{
	if (p_byteCode.empty())
		return VkShaderModule(); //bad :(

	VkShaderModuleCreateInfo shaderModuleCreateInfo{};

	shaderModuleCreateInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = p_byteCode.size();
	shaderModuleCreateInfo.pCode	= (uint32_t*)p_byteCode.data();

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(this->mLogicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) == VK_SUCCESS)
		return shaderModule;
	else
		return VkShaderModule(); //I really dont know what to do with that
}

bool VKRenderer::CreateSyncObjects()
{
	mRenderingSemaphore.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);
	mImageAviableSemaphore.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);
	mPresentFence.resize(this->mGraphicsPipeline.MAX_CONCURENT_FRAMES);

	VkSemaphoreCreateInfo semaphoreCreateInfo{};

	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceCreateInfo{};

	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(int i = 0; i < this->mGraphicsPipeline.MAX_CONCURENT_FRAMES; i++)
	{ 
		vkCreateSemaphore(this->mLogicalDevice, &semaphoreCreateInfo, nullptr, &this->mImageAviableSemaphore[i]);
		vkCreateSemaphore(this->mLogicalDevice, &semaphoreCreateInfo, nullptr, &this->mRenderingSemaphore[i]);
		vkCreateFence(this->mLogicalDevice, &fenceCreateInfo, nullptr, &this->mPresentFence[i]);
	}

	return true;
}

bool VKRenderer::CreateBuffer(VkDeviceSize p_size, VkBufferUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkBuffer& p_buffer, VkDeviceMemory& p_bufferMemory)
{
	VkBufferCreateInfo bufferCreateInfo{};

	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = p_size;
	bufferCreateInfo.usage = p_usage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	bool result = vkCreateBuffer(this->mLogicalDevice, &bufferCreateInfo, nullptr, &p_buffer) == VK_SUCCESS;

	VkMemoryRequirements memoryRequirements;

	vkGetBufferMemoryRequirements(this->mLogicalDevice, p_buffer, &memoryRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo{};

	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = this->FindMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	result &= vkAllocateMemory(this->mLogicalDevice, &memoryAllocateInfo, nullptr, &p_bufferMemory) == VK_SUCCESS;

	vkBindBufferMemory(this->mLogicalDevice, p_buffer, p_bufferMemory, 0);

	return result;
}

bool VKRenderer::CreateImage(uint32_t p_width, uint32_t p_height, VkFormat p_format, VkImageTiling p_tiling, VkImageUsageFlags p_usage, VkMemoryPropertyFlags p_properties, VkImage& p_image, VkDeviceMemory& p_imageMemory)
{
	bool result = false;

	VkImageCreateInfo imageCreateInfo{};

	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = p_width;
	imageCreateInfo.extent.height = p_height;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.format = p_format;
	imageCreateInfo.tiling = p_tiling;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = p_usage;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	result = vkCreateImage(this->mLogicalDevice, &imageCreateInfo, nullptr, &p_image) == VK_SUCCESS;

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(this->mLogicalDevice, p_image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = this->FindMemoryType(memRequirements.memoryTypeBits, p_properties);

	result &= vkAllocateMemory(this->mLogicalDevice, &allocInfo, nullptr, &p_imageMemory) == VK_SUCCESS;

	vkBindImageMemory(this->mLogicalDevice, p_image, p_imageMemory, 0);

	return result;
}

bool VKRenderer::CreateIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(this->indices[0]) * this->indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	bool result = this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory) == VK_SUCCESS;

	void* data;
	vkMapMemory(this->mLogicalDevice , stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, this->indices.data(), (size_t)bufferSize);
	vkUnmapMemory(this->mLogicalDevice, stagingBufferMemory);

	result &= this->CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->mIndexBuffer, this->mIndexBufferMemory) == VK_SUCCESS;

	this->CopyBuffer(stagingBuffer, this->mIndexBuffer, bufferSize);

	vkDestroyBuffer(this->mLogicalDevice, stagingBuffer, nullptr);
	vkFreeMemory(this->mLogicalDevice, stagingBufferMemory, nullptr);

	return result;
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
	result &= this->CreateDepthRessources();
	result &= this->CreateDescriptorSetLayout();
	result &= this->SetupGraphicsPipeline();
	result &= this->CreateFrameBuffers();
	result &= this->CreateCommandBuffer();
	result &= this->CreateTextureImage();
	result &= this->CreateTextureImageView();
	result &= this->CreateTextureSampler();
	result &= this->CreateVertexBuffer();
	result &= this->CreateIndexBuffer();
	result &= this->CreateUniformBuffers();
	result &= this->CreateDescriptorPool();
	result &= this->CreateDescriptorSets();
	result &= this->CreateSyncObjects();

	return result;
}

void VKRenderer::Release()
{
	vkDeviceWaitIdle(this->mLogicalDevice); //Smol security

	//Sync objects
	for (int i = 0; i < this->mGraphicsPipeline.MAX_CONCURENT_FRAMES; i++)
	{ 
		vkDestroySemaphore(this->mLogicalDevice, this->mImageAviableSemaphore[i], nullptr);
		vkDestroySemaphore(this->mLogicalDevice, this->mRenderingSemaphore[i], nullptr);
		vkDestroyFence(this->mLogicalDevice, this->mPresentFence[i], nullptr);
	}

	//Vertex Buffer
	vkDestroyBuffer(this->mLogicalDevice, this->mVertexBuffer, nullptr);
	vkFreeMemory(this->mLogicalDevice, this->mVertexBufferMemory, nullptr);

	//Index Buffer
	vkDestroyBuffer(this->mLogicalDevice, this->mIndexBuffer, nullptr);
	vkFreeMemory(this->mLogicalDevice, this->mIndexBufferMemory, nullptr);

	//Texture
	vkDestroySampler(this->mLogicalDevice, this->mTextureSampler, nullptr);
	vkDestroyImageView(this->mLogicalDevice, this->mTextureImageView, nullptr);
	vkDestroyImage(this->mLogicalDevice, this->mTextureImage, nullptr);
	vkFreeMemory(this->mLogicalDevice, this->mTextureImageMemory, nullptr);

	//Command buffer
	vkFreeCommandBuffers(this->mLogicalDevice, this->mCommandPool, this->mGraphicsPipeline.MAX_CONCURENT_FRAMES, this->mCommandBuffer.data());
	vkDestroyCommandPool(this->mLogicalDevice, this->mCommandPool, nullptr);

	//Framebuffers
	for (VkFramebuffer frameBuffer : this->mSwapChain.frameBuffers)
		vkDestroyFramebuffer(this->mLogicalDevice, frameBuffer, nullptr);

	//Depth ressources
	vkDestroyImage(this->mLogicalDevice, this->mDepthRessources.depthImage, nullptr);
	vkDestroyImageView(this->mLogicalDevice, this->mDepthRessources.depthImageView, nullptr);
	vkFreeMemory(this->mLogicalDevice, this->mDepthRessources.depthMemory, nullptr);

	//Descriptors
	vkDestroyDescriptorSetLayout(this->mLogicalDevice, this->mDescriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(this->mLogicalDevice, this->mDescriptorPool, nullptr);
	for (size_t i = 0; i < this->mGraphicsPipeline.MAX_CONCURENT_FRAMES; i++)
	{
		vkDestroyBuffer(this->mLogicalDevice, this->mUniformBuffers[i], nullptr);
		vkFreeMemory(this->mLogicalDevice, this->mUniformBuffersMemory[i], nullptr);
	}

	//Pipeline
	vkDestroyPipeline(this->mLogicalDevice, this->mGraphicsPipeline.vkPipeline, nullptr);
	vkDestroyPipelineLayout(this->mLogicalDevice, this->mGraphicsPipeline.vkPipelineLayout, nullptr);
	vkDestroyRenderPass(this->mLogicalDevice, this->mGraphicsPipeline.vkRenderPass, nullptr);

	//shader modules
	vkDestroyShaderModule(this->mLogicalDevice, this->mFragmentShader, nullptr);
	vkDestroyShaderModule(this->mLogicalDevice, this->mVertexShader, nullptr);

	//Swapchain
	for (const VkImageView& imageView : this->mSwapChain.imageViews)
		vkDestroyImageView(this->mLogicalDevice, imageView, nullptr);
	vkDestroySwapchainKHR(this->mLogicalDevice, this->mSwapChain.vkSwapChain, nullptr);

	//Other
	vkDestroySurfaceKHR(this->mVKInstance, this->mRenderingSurface, nullptr);

	//
	//Always destory device/instance at the very end !
	//

	vkDestroyDevice(this->mLogicalDevice, nullptr);

	vkDestroyInstance(this->mVKInstance, nullptr);
}

void VKRenderer::UpdateUniformBuffer()
{
	UniformBufferObject ubo{};

	ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), this->mSwapChain.extent.width / (float)this->mSwapChain.extent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	memcpy(this->mUniformBuffersMap[this->mCurrentFrame], &ubo, sizeof(ubo));
}

void VKRenderer::Render()
{
	//
	//Frame prep
	//

	vkWaitForFences(this->mLogicalDevice, 1, &this->mPresentFence[this->mCurrentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(this->mLogicalDevice, 1, &this->mPresentFence[this->mCurrentFrame]);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(this->mLogicalDevice, this->mSwapChain.vkSwapChain, UINT64_MAX, this->mImageAviableSemaphore[this->mCurrentFrame], VK_NULL_HANDLE, &imageIndex);

	vkResetCommandBuffer(this->mCommandBuffer[this->mCurrentFrame], 0);

	//
	//Update objects data (idealy in engine class)
	//

	UpdateUniformBuffer();

	//
	//Command Buffer
	//

	this->RecordCommandBuffer(this->mCommandBuffer[this->mCurrentFrame], imageIndex);

	VkSubmitInfo submitInfo{};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->mImageAviableSemaphore[this->mCurrentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->mCommandBuffer[this->mCurrentFrame];

	VkSemaphore signalSemaphores[] = { this->mRenderingSemaphore[this->mCurrentFrame] };

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkQueueSubmit(this->mGraphicsQueue, 1, &submitInfo, this->mPresentFence[this->mCurrentFrame]);
	
	//
	//Present stuff
	//

	VkPresentInfoKHR presentInfo{};

	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->mSwapChain.vkSwapChain };

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	vkQueuePresentKHR(this->mPresentQueue, &presentInfo);

	this->mCurrentFrame = (this->mCurrentFrame + 1) % this->mGraphicsPipeline.MAX_CONCURENT_FRAMES;
}