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
	applicationInfo.apiVersion = VK_API_VERSION_1_1;

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


	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};

	pipelineVertexInputStateCreateInfo.sType						 = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;


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
	pipelineRasterizationStateCreateInfo.frontFace					= VK_FRONT_FACE_CLOCKWISE;
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
		
	this->mShaderStages = { vertShaderStageInfo, fragShaderStageInfo };

	//
	//Pipeline layout
	//

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

	if (vkCreatePipelineLayout(this->mLogicalDevice, &pipelineLayoutInfo, nullptr, &this->mPipelineLayout) != VK_SUCCESS)
		return false;

	//
	//Render pass
	//

	VkAttachmentReference subPassColorAttachement{};

	subPassColorAttachement.attachment = 0;
	subPassColorAttachement.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpassDescription{};
	
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pipelineBindPoint	= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments	= &subPassColorAttachement;

	VkAttachmentDescription colorAttachment{};
	
	colorAttachment.format			= this->mSwapChain.imageFormat;
	colorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


	VkRenderPassCreateInfo renderPassCreateInfo{};

	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &colorAttachment;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;

	if (vkCreateRenderPass(this->mLogicalDevice, &renderPassCreateInfo, nullptr, &this->mRenderPass) != VK_SUCCESS)
		return false;

	//
	//Graphics pipeline object
	//

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};

	graphicsPipelineCreateInfo.sType		= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.layout		= this->mPipelineLayout;
	graphicsPipelineCreateInfo.renderPass	= this->mRenderPass;
	graphicsPipelineCreateInfo.subpass		= 0;
	graphicsPipelineCreateInfo.stageCount	= 2;
	graphicsPipelineCreateInfo.pStages		= this->mShaderStages.data();

	graphicsPipelineCreateInfo.pDynamicState		= &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.pVertexInputState	= &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState	= &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState		= &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState	= &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState	= &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState		= &pipelineColorBlendStateCreateInfo;

	graphicsPipelineCreateInfo.pDepthStencilState = nullptr;
	graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	graphicsPipelineCreateInfo.basePipelineIndex = -1;

	return vkCreateGraphicsPipelines(this->mLogicalDevice, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &this->mGraphicsPipeline) == VK_SUCCESS;
}

bool VKRenderer::CreateFrameBuffers()
{
	this->mFrameBuffers.resize(this->mSwapChain.imageViews.size());

	for (int i = 0; i < mFrameBuffers.capacity(); i++)
	{
		VkImageView attachments[] = {
			this->mSwapChain.imageViews[i]
		};


		VkFramebufferCreateInfo framebufferCreateInfo{};

		framebufferCreateInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.height		= this->mSwapChain.extent.height;
		framebufferCreateInfo.width			= this->mSwapChain.extent.width;
		framebufferCreateInfo.renderPass	= this->mRenderPass;
		framebufferCreateInfo.layers		= 1;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.pAttachments  = attachments;

		vkCreateFramebuffer(this->mLogicalDevice , &framebufferCreateInfo, nullptr, &this->mFrameBuffers[i]) == VK_SUCCESS;
	}

	return true;
}

bool VKRenderer::CreateCommandBuffer()
{
	//
	//Command pool 
	//

	VkCommandPoolCreateInfo commandPoolCreateInfo{};

	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = this->mPhysicalDevice.supportedQueues.graphicsFamily;

	bool result = vkCreateCommandPool(this->mLogicalDevice, &commandPoolCreateInfo, nullptr, &this->mCommandPool) == VK_SUCCESS;

	//
	//Command buffer
	//

	//result &= vkAllocateCommandBuffers();

	return result;
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
	result &= this->SetupGraphicsPipeline();
	result &= this->CreateFrameBuffers();

	return result;
}

void VKRenderer::Release()
{
	//Command buffer
	vkDestroyCommandPool(this->mLogicalDevice, this->mCommandPool, nullptr);

	//Framebuffers
	for (VkFramebuffer frameBuffer : this->mFrameBuffers)
		vkDestroyFramebuffer(this->mLogicalDevice, frameBuffer, nullptr);

	//Pipeline
	vkDestroyPipeline(this->mLogicalDevice, this->mGraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(this->mLogicalDevice, this->mPipelineLayout, nullptr);
	vkDestroyRenderPass(this->mLogicalDevice, this->mRenderPass, nullptr);

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
	//Always destory instance/device at the very end !
	//

	vkDestroyDevice(this->mLogicalDevice, nullptr);

	vkDestroyInstance(this->mVKInstance, nullptr);
}