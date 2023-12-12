/**************************************************************************//**
*	@file   Stub.cpp
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		Implementation of the HelloTriangleApplication class
* 
*		All code is referenced from: https://vulkan-tutorial.com/
******************************************************************************/

#include "HelloTriangleApplication.h"

#include <stdexcept>
#include <iostream>
#include <map>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>

// Window const sizes
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Vulkan validation layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Vulkan device extensions. Define required extensions of a device
const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// Macros for if the validation layers are on or off
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// Helper function for reading binary files
static std::vector<char> readFile(const std::string& filename) {
	// Open the filename in a binary format (ate - read from end of file)
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	// If the file is not open, throw
	if (!file.is_open()) {
		throw std::runtime_error("Failed to Open File!");
	}

	// Get what the file size is and create a buffer with the file info
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Now go back to the beginning and read in all the data
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	// Remember to close the file and return the buffer
	file.close();
	return buffer;
}

// Function for creating debug messenger (checks if the layer is available or not)
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

// Function for destroying the debug messenger (Checked if layer is available or not)
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

// Function for running the application
void HelloTriangleApplication::run() {
	initWindow();
	initVulkan();
	mainloop();
	cleanup();
}

// Initialize glfw and create the window
void HelloTriangleApplication::initWindow() {
	// Initialize glfw
	glfwInit();

	// Tell GLFW to not create a gl context and no resizable window
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Create the GLFW window
	window_ = glfwCreateWindow(WIDTH, HEIGHT, "My Vulkan Window", nullptr, nullptr);
}

// Initialize vulkan
void HelloTriangleApplication::initVulkan() {
	// To initialize Vulkan, we need an instance
	createInstance();

	// Also setup debug messenger
	setupDebugMessenger();

	// Create the rendering surface
	createSurface();

	// Pick the physical device
	pickPhysicalDevice();

	// Create a logical device
	createLogicalDevice();
	
	// Create a swap chain
	createSwapChain();

	// Create the image viewers
	createImageViews();

	// Create a render pass
	createRenderPass();

	// Create a graphics pipeline
	createGraphicsPipeline();

	// Create frame buffers
	createFramebuffers();

	// Create the command pool and a command buffer
	createCommandPool();
	createCommandBuffer();
}

// Create the vulkan instance
void HelloTriangleApplication::createInstance() {
	// Before creating an instance, check for validation layers
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("Validation layers requested, but not available!");
	}

	// Create an application info structure for Vulkan
	// (This is technically optional, however we will still use it
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "My Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Now create a create info struct for Vulkan
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Need to get what extensions are required for glfw and Vulkan
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Checking for if we have set up the debug messenger
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	// Now we can actually create a vk instance. Remember to error check
	VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance_);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}

	// There is a discussion about how to get available Vulkan extensions, but will not
	// implement at the moment
}

// Validation layer support
bool HelloTriangleApplication::checkValidationLayerSupport() {
	// First get how many layers are available
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	// Get all the available layers
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// Check if all layers in validation layers are in available layers
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

// Get required extensions based on validation layers being enabled
std::vector<const char*> HelloTriangleApplication::getRequiredExtensions() {
	// Get all the required glfw extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// If we are using validation layers, there is one additional layer we need
	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void HelloTriangleApplication::setupDebugMessenger() {
	// Validation Layers are not enabled, so we don't need to do anything
	if (!enableValidationLayers) return; 

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	// Attempt to create the debug messenger
	if (CreateDebugUtilsMessengerEXT(vkInstance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to set up debug messenger");
	}
}

// loop for the application
void HelloTriangleApplication::mainloop() {
	// Keep processing all the inputs from the glfw window
	while (!glfwWindowShouldClose(window_)) {
		glfwPollEvents();
	}
}

// Cleanup function
void HelloTriangleApplication::cleanup() {
	// Note: The VkPhysicalDevice is destroyed when the instance is destroyed
	//	so we don't need to worry about it

	// Don't forget to destroy the command pool
	vkDestroyCommandPool(logicalDevice_, commandPool_, nullptr);

	// Destroy the frame buffers
	for (auto framebuffer : swapChainFramebuffers_) {
		vkDestroyFramebuffer(logicalDevice_, framebuffer, nullptr);
	}

	// Destroy the graphics pipeline
	vkDestroyPipeline(logicalDevice_, gfxPipeline_, nullptr);

	// Destroy the pipeline layout
	vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);

	// Destroy the render passs
	vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);

	// Clean up the image views
	for (auto imageView : swapChainImageViews_) {
		vkDestroyImageView(logicalDevice_, imageView, nullptr);
	}

	// Destroy the swapchain
	vkDestroySwapchainKHR(logicalDevice_, swapChain_, nullptr);

	// Don't forget to destroy the logical device
	vkDestroyDevice(logicalDevice_, nullptr);

	// Destroy the debug messenger if validation layers are enabled
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vkInstance_, debugMessenger_, nullptr);
	}

	// Don't forget to destroy the surface
	vkDestroySurfaceKHR(vkInstance_, surface_, nullptr);

	// Destroy the vulkan instance
	vkDestroyInstance(vkInstance_, nullptr);

	// Destroy the glfw window
	glfwDestroyWindow(window_);

	// Terminate glfw
	glfwTerminate();
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

// Check if a physical device is suitable for this application
bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device) {
	// Check the queue families and see if the device is suitable
	QueueFamilyIndices indices = findQueueFamilies(device);

	// Also get if the needed extensions are supported
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// Check if the device has good enough swap chain support
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		// Check that the swap chain has actual formats and presentation modes
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// Check that everything here is good
	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device) {
	// Get all the available extensions of this device
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// Create a set of all the required extensions
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	// Iterate over all the available extensions and remove from the set any that match
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	// If set is empty, all extensions supported
	return requiredExtensions.empty();
}

// Function to help Vulkan pick a physical device
void HelloTriangleApplication::pickPhysicalDevice() {
	// Get how many physical devices are available
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(vkInstance_, &deviceCount, nullptr);

	// If there are no devices, don't even continue
	if (deviceCount == 0)
		throw std::runtime_error("Failed to find GPUs with Vulkan Support!");

	// List out all the available physical devices
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(vkInstance_, &deviceCount, devices.data());

	// Rate the device suitability
	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : devices) {
		int score = rateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check best candidate
	if (candidates.rbegin()->first > 0) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(candidates.rbegin()->second, &deviceProperties);

		std::cout << "Selecting Device: " << deviceProperties.deviceName << std::endl;

		vkPhysicalDevice_ = candidates.rbegin()->second;
	}
	else {
		throw std::runtime_error("Failed to find a suitable GPU");
	}
}

int HelloTriangleApplication::rateDeviceSuitability(VkPhysicalDevice device) {
	// Query the device properties
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Query the device features
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	
	// Score of the device
	int score = 0;

	// Discrete GPUs are desireable
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Max size for textures
	score += deviceProperties.limits.maxImageDimension2D;

	// Application can't run without shaders
	if (!deviceFeatures.geometryShader)
		return 0;

	return score;
}

// Finding queue families
QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device) {
	// Create a structure for the indices
	QueueFamilyIndices indices;

	// Find the queue families
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Find one family that supports graphics
	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		// We want the graphics bit
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		// Check for surface support
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
		if (presentSupport)
			indices.presentFamily = i;

		// If we already found the graphics bit, break from loop
		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

void HelloTriangleApplication::createLogicalDevice() {
	// Get the queue families
	QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice_);

	// Need multiple create structs for multiple queue families
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		// Now fill a structure for creating the device
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Define device features. We will come back to this later
	VkPhysicalDeviceFeatures deviceFeatures{};
	
	// Now moving on to creating the actual logical device
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Specify the queue creation info and what features to enable
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// Specify any alidation layers
	createInfo.enabledExtensionCount = 0;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// Specify any extensions
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Now create the device
	if (vkCreateDevice(vkPhysicalDevice_, &createInfo, nullptr, &logicalDevice_) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device!");

	// Get the graphics queue and present queue from the device
	vkGetDeviceQueue(logicalDevice_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
	vkGetDeviceQueue(logicalDevice_, indices.presentFamily.value(), 0, &presentQueue_);
}

void HelloTriangleApplication::createSurface() {
	if (glfwCreateWindowSurface(vkInstance_, window_, nullptr, &surface_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create window surface.");
	}
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device) {
	// Get swap chain support details 
	SwapChainSupportDetails details;

	// Get surface capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

	// Get the supported surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
	}

	// Get the supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
	}

	// Return all the polled details
	return details;
}

// Select the swap chain format
VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	// Iterating over all the available formats, choose the first one that:
	// Supports BGRA 32-bit and Supports SRGB
	for (const auto& format : availableFormats) {
		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	// If it isn't found, just return the first available format
	return availableFormats[0];
}

// Select the presentation mode
VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	// Four different kinds of presentation modes:
	// Immediate
	// FIFO
	// FIFO relaxed
	// mailbox (triple buffering)
	// Ideal is mailbox if energy consumption is not a concern

	// Find that one of the available modes is mailbox.
	for (const auto& mode : availablePresentModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return mode;
		}
	}

	// Otherwise, FIFO is guaranteed
	return VK_PRESENT_MODE_FIFO_KHR;
}

// Select the swap extent
VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	// Resolution of swap chain images, usually equal to resolution of the window
	// we are drawing to in pixels

	// If the width is not max, the resolution is already specified
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	// Otherwise, we need to figure out the extent ourselves.
	// Get the width and heigh of the window glfw created
	int width, height;
	glfwGetFramebufferSize(window_, &width, &height);

	VkExtent2D actualExtent = {
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	};

	// Don't forget to clamp the width and height based on min and max image extents
	actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	// Return the newly calculated extent
	return actualExtent;
}

// Create a swap chain
void HelloTriangleApplication::createSwapChain() {
	// Get swap chain support details
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(vkPhysicalDevice_);

	// Figure out what surface format, presentation mode, and extents we will use
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extents = chooseSwapExtent(swapChainSupport.capabilities);

	// Also figure out how many images will be in the swap chain
	// recommended to always get one more than the minimum
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// Also make sure the maximum is not being exceeded
	// (A maximum of zero means no limit)
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Now to start creating the swap chain creation struct
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface_;

	// details of the swap chain
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extents;

	// This value is almost always one, unless there are special cases
	createInfo.imageArrayLayers = 1;

	// Rendering directly to these images, so used as color attachments
	// Other options that may be useful
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(vkPhysicalDevice_);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	// Specify how to handle swap chain images across multiple queue families
	// Setting up concurrent so that we don't need to set up ownership stuff.
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;     // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	// Transformations can be applied to the images if desired. We don't want this, so set to itself
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

	// don't use alpha for blending between windows. Ignore the alpha then
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	// Set the presentation mode and clip pixels that can't be seen
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	
	// Null for now, but needs to be handled if the screen is resized
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Now try to create the swapchain
	if (vkCreateSwapchainKHR(logicalDevice_, &createInfo, nullptr, &swapChain_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swap chain!");
	}

	// With the swapchain created, get the handles of the images in the swapchain
	vkGetSwapchainImagesKHR(logicalDevice_, swapChain_, &imageCount, nullptr);
	swapChainImages_.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice_, swapChain_, &imageCount, swapChainImages_.data());

	// Don't forget to save the format and the chain extent
	swapChainImageFormat_ = surfaceFormat.format;
	swapChainExtent_ = extents;
}

void HelloTriangleApplication::createImageViews() {
	// Start with resizing the vector to how many images are in the swapchain
	swapChainImageViews_.resize(swapChainImages_.size());

	// Iterate over the swap chain images
	for (size_t i = 0; i < swapChainImages_.size(); ++i) {
		// Start a creation structure for an image view object
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages_[i];

		// Specify view type and give format from swap chain
		// View Type will almost always be 2D textures
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat_;

		// Components lets you swizzle the colors around.
		// Stick with default mapping
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Subresource Range is the purpose of the image.
		// Used as color targets with no mipmapping levels or multiple layers
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		// Now call the create function for the object
		if (vkCreateImageView(logicalDevice_, &createInfo, nullptr, &swapChainImageViews_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image views!");
		}
	}
}

void HelloTriangleApplication::createGraphicsPipeline() {
	// Pull the code 
	auto vertShaderCode = readFile("data/shaders/vert.spv");
	auto fragShaderCode = readFile("data/shaders/frag.spv");

	// Create the shader modules
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// Creation info for the vertex stage
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main"; // Entrypoint of the shader
	// pSpecializationInfo allows specifying values for shader constants

	// Creation info for the fragment stage
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main"; // Entrypoint of the shader

	// Now create an array with these shader stages
	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	//****************************************************************************
	//	Dynamic State Creation information
	//****************************************************************************
	std::vector<VkDynamicState> dynamicStates = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	//****************************************************************************
	//	Vertex Input State Creation information
	//****************************************************************************
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	// Spacing between data and whether data is per-vertex or per-instance (instancing)
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;   // Optional

	// Type of attributes passed to vertex shader, what binding they associate to, and at which offset.
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	//****************************************************************************
	//	Input Assembly State Creation information
	//****************************************************************************
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;

	// What kind of geometry will be drawn. Examples: Point List, Line List, Line Strip, triangle list, triangle strip, etc.
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// if true, can break up inputs for the Strip topologies
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	//****************************************************************************
	//	Viewports and Scissors
	//****************************************************************************
	VkViewport viewport{};
	
	// These will almost always be the values for a viewport
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent_.width;
	viewport.height = (float)swapChainExtent_.height;

	// These must always be between 0 and 1
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor will discard any pixels outside of a range
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent_;

	// We can make viewport and scissors static or dynamic. Dynamic is more desireable, but will be static for now
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//****************************************************************************
	//	Rasterizer
	//		Vertex to Fragment. Runs Depth tests, face culling, and scissor test.
	//		Can also output fragments that fill entire polys or just edges, like
	//			wireframe rendering.
	//****************************************************************************
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

	// If true, fragments outside near and far planes are clamped instead of discarded
	// Useful for shadow maps. Required a GPU feature to be enabled
	rasterizer.depthClampEnable = VK_FALSE;

	// If true, geometry never passed through rasterizer stage. Essentially disabled output to framebuffer
	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	// How fragments are generated for geometry. Modes available are:
	//	Fill - fill area of polygon with fragments
	//	Line - polygon edges are drawn as lines
	//	Point - polygon vertices are drawn as points
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	// Straight forward. Any line thicker than 1.0f required wideLines GPU feature enabled
	rasterizer.lineWidth = 1.0f;

	// Cull mode and what the rasterizer considered front facing
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	// TODO: THIS SHOULD BE COUNTER CLOCK-WISE. RIGHT HAND RULE!!!
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	// Rasterizer can change depth values by biasing them with constant values
	//	or biasing them based on a fragment's slope
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional;

	//****************************************************************************
	//	Multisampling
	//		Helps with Anti-Aliasing. Combines fragment shader results of
	//		multiple polygons that rasterize to same pixel.
	// 
	//		We will return to this later
	//****************************************************************************
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	//****************************************************************************
	//	Depth and Stencil Testing
	//		We will return to this later
	//****************************************************************************

	//****************************************************************************
	//	Color Blending
	//		Returned color from fragment shader needs to be combined with the
	//		color already in the FrameBuffer. Two ways to do this:
	//			1. Mix old and new value for a final color
	//			2. Combine the old and new value using a bitwise operation
	//		There are also two struct types for color blending:
	//			1. Attachment State, which configures per each frame buffer.
	//			2. CreateInfo, which configured global color blending
	//****************************************************************************

	// We only have one Frame Buffer, so we will create one Attachment State
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	// Pseudo code of how the blending works:
	/*
		if (blendEnable) {
			finalColor.rgb = (srcColorBlendFactor * newColor.rgb) < colorBlendOp > (dstColorBlendFactor * oldColor.rgb);
			finalColor.a = (srcAlphaBlendFactor * newColor.a) < alphaBlendOp > (dstAlphaBlendFactor * oldColor.a);
		}
		else {
			finalColor = newColor;
		}

		finalColor = finalColor & colorWriteMask;
	*/
	// This implements alpha blending
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;           // Optional | Factor of using the new color
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional | Factor of using the old color
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional | What operation to do when blending the colors
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;  // Optional | Factor of using the new alpha
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional | Factor of using the old alpha
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional | What operation to do when blending the alpha

	// Now we create the actual blend state
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;

	// Allows the use of bitwise combination for blending. 
	// Note: If enabled, will treat blendEnable as VK_FALSE
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional

	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	//****************************************************************************
	//	Pipeline Layout
	//		Uniform variables must be specified at creation time of a pipeline
	//		Even if you don't have uniform variables, you are required to create
	//		this layout
	//****************************************************************************
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(logicalDevice_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	//****************************************************************************
	//	Graphics Pipeline
	//		Now that we have everything set up, we can actually create a
	//		graphics pipeline
	//****************************************************************************
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

	// Specify how many shader stages there will be
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	// Specify all the fixed functions
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;

	// Specify the pipeline layout (uniform variables)
	pipelineInfo.layout = pipelineLayout_;

	// Specify the render pass and subpasses
	pipelineInfo.renderPass = renderPass_;
	pipelineInfo.subpass = 0; // Index of the subpass

	// This allows for deriving a pipeline from a base pipeline
	// Also only used if VK_PIPELINE_CREATE_DERIVATIVE_BIT flag is specified in create info
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	// Now create the graphics pipeline
	if (vkCreateGraphicsPipelines(logicalDevice_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &gfxPipeline_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Graphics Pipeline!");
	}
	
	// Don't forget to destroy the shader modules
	vkDestroyShaderModule(logicalDevice_, fragShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice_, vertShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code) {
	// Start the creation information of the shader module
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	// Create the shader module
	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}

	// Return the shader module
	return shaderModule;
}

//*****************************************************************************
//	Create Render Pass
//		Tell Vulkan about the framebuffer attachments that will be used
//		while rendering.
//		- How many color buffers
//		- How many depth buffers
//		- How many samples for each
//		- How to handle contents through rendering
//*****************************************************************************
void HelloTriangleApplication::createRenderPass() {
	//****************************************************************************
	//	Color Attachment
	//		Describe the color attachment in the render pass
	//****************************************************************************
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat_;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	// What to do with the data in attachment before rendering:
	//	LOAD - Preserve existing contents of attachment
	//	CLEAR - Clear values to a constant
	//	DONT_CARE - Existing contents undefined
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

	// What to do with the data in attachment after rendering
	//	STORE - Rendered contents stored in memory and can be read later
	//	DONT_CARE - Contents of framebuffer undefined after rendering operation
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	// Applies to stencil data. Since we are not using stencil, don't care.
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Layout of pixels in memory can change based on what you are doing with an image
	// Common Layouts:
	//	COLOR_ATTACHMENT_OPTIMAL - Image is used as a color attachment
	//	PRESENT_SRC_KHR - Image is presented in the swap chain
	//	TRANSFER_DST_OPTIMAL - Image is used as destination for memory copy operation

	// Layout of the image before render pass starts
	// We don't care what the image layout was before, so UNDEFINED
	// Undefined does mean that contents are not guaranteed to be preserved, but we are clearing it anyways
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Layout of the image after render pass
	// We want the image to be ready for presentation, so PRESENT_SRC_KHR
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//****************************************************************************
	//	Subpasses & Attachment Refs.
	//		A renderpass can have multiple subpasses, like post-processing
	//		that happens one after another. 
	// 
	//		Just one subpass for our triangle
	//****************************************************************************

	// Each subpass references one or more attachments
	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0; // We only have one attachment, so index is zero
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Attachment is a color buffer

	// Subpass Description, make sure it is known as a graphics subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// Reference the color attachment
	// Fragment shader refers to the attachment location in the shader! (out vec4 outColor)
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	// Other attachments that can be referenced by a subpass include:
	//	pInputAttachments - Attachments read from a shader
	//	pResolveAttachments - used for multisampling color attachments
	//	pDepthStencilAttachment - from depth and stencil data
	//	pPreserveAttachments - not used by this subpass, but data must be preserved

	//****************************************************************************
	//	Render Pass
	//		Now with everything specified, we can create the render pass
	//****************************************************************************
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	if (vkCreateRenderPass(logicalDevice_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

//*****************************************************************************
//	Creation of frame buffers
//*****************************************************************************
void HelloTriangleApplication::createFramebuffers() {
	// Resize container 
	swapChainFramebuffers_.resize(swapChainImageViews_.size());

	// Iterate over the image views and make framebuffers for them
	for (size_t i = 0; i < swapChainImageViews_.size(); ++i) {
		// Array for the frame buffer to know that vk images they should be bound to
		VkImageView attachments[] = {
			swapChainImageViews_[i]
		};

		// Creation struct for a frame buffer
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

		// Needs to know what render pass it is using
		framebufferInfo.renderPass = renderPass_;

		// How many attachments is this frame buffer using
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;

		// Width and height of the framebuffer
		framebufferInfo.width = swapChainExtent_.width;
		framebufferInfo.height = swapChainExtent_.height;

		// Images in swapchain are single images, so layers is one
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(logicalDevice_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Framebuffer!");
		}
	}
}

//*****************************************************************************
//	Creation of command pool
//*****************************************************************************
void HelloTriangleApplication::createCommandPool() {
	// We will need the queue family indices
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vkPhysicalDevice_);

	// Creation structure
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

	// Two options:
	//	TRANSIENT_BIT - Command buffers rerecorded with new commands very often
	//	RESET_COMMAND_BUFFER_BIT - Command buffers rerecorded individually. Without flag, all reset together
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	// Command buffers executed by submitting to one of the device queues.
	// Going to record commands for drawing, so choose graphics queue family
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	// Actually create the command pool
	if (vkCreateCommandPool(logicalDevice_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create command pool!");
	}
}

//*****************************************************************************
//	Creation of command buffer
//*****************************************************************************
void HelloTriangleApplication::createCommandBuffer() {
	// Creation struct
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

	// Command buffer needs to allocate from a pool
	allocInfo.commandPool = commandPool_;

	// Two types:
	//	PRIMARY - Can be submitted to a queue for execution, not not called from other command buffers
	//	SECONDARY - Can't be submitted directly, but can be called from primary command buffers
	// Useful for common operations that are always reused
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	// We are only allocating one command buffer
	allocInfo.commandBufferCount = 1;

	// Now actually create the command buffer
	if (vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &commandBuffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

//*****************************************************************************
//	Record Command Buffer
//		Writes commands we want to execute into a command buffer
//*****************************************************************************
void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Options are:
	//	ONE_TIME_SUBMIT_BIT - Command buffer will be rerecorded right after executing once
	//	RENDER_PASS_CONTINUE_BIT - Secondary command buffer that will be within a single render pass
	//	SIMULTANEOUS_USE_BIT - Command buffer can be resubmitted while already pending execution
	beginInfo.flags = 0; // Optional

	// Only needed for secondary command buffers
	beginInfo.pInheritanceInfo = nullptr; // Optional

	// Begin the command buffer
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		throw std::runtime_error("Failed to begin recording command buffer!");
	}

	//*****************************************************************************
	//	Start a render pass
	//		Drawing starts by starting a render pass
	//*****************************************************************************
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

	// What render pass to use, and what frame buffer to use
	renderPassInfo.renderPass = renderPass_;
	renderPassInfo.framebuffer = swapChainFramebuffers_[imageIndex];

	// Define the render area, should match the size of attachments for best performance
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainExtent_;

	// What color to clear with, just go with black to start
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	// Now actually begin the render pass
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// vkCmd prefix is a function that records a command
	// First parameter for a command is always the command buffer to record command to
	// Seconds parameter specified details of the render pass
	// Final parameter controlls how commands within render pass will be provided
	// Two values:
	//	INLINE - embedded in the primary command buffer itself & no secondary command buffer execution
	//	SECONDARY_COMMAND_BUFFERS - Render pass commands will be executed from secondary command buffers

	//*****************************************************************************
	//	Some basic drawing commands
	//*****************************************************************************
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gfxPipeline_);

	// Viewport and scissor are dynamic, so need to specify here
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent_.width);
	viewport.height = static_cast<float>(swapChainExtent_.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = swapChainExtent_;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Now with those set, we can call the command to draw our triangles
	// Parameters:
	//	The command buffer to record to
	//	How many vertices to render
	//	How many instances to render
	//	Offset into vertex buffer, defines lowest value of gl_VertexIndex
	//	Offset for instanced rendering, defined lowest value of gl_InstanceIndex
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	// Now we end the render pass
	vkCmdEndRenderPass(commandBuffer);

	// Finish rerecording the command buffer
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	// Can use messageSeverity to determine if the message is important
	// enough to be shown or not

	// messageType is fairly straight forward

	// CallbackData has the actual message to display
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	// Should almost always return VK_FALSE
	return VK_FALSE;
} 
