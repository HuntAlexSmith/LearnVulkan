/**************************************************************************//**
*	@file   Stub.cpp
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		Implementation of the HelloTriangleApplication class
* 
*		All code is referenced from: https://vulkan-tutorial.com/
******************************************************************************/

//*****************************************************************************
//	CHALLENGE:
//		Specify a Transfer queue that is not the Graphics queue or the compute
//		queue.
//	The graphics queue already handles memory transfers, but then you have
//		a queue being used for multiple purposes. May be a good idea to have
//		a separate queue that is purely for transfering buffers over from
//		CPU to GPU
//*****************************************************************************

#include "HelloTriangleApplication.h"

#include <stdexcept>
#include <iostream>
#include <map>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <chrono>

// Hard codes vertices for our mesh
const std::vector<Vertex> vertices = {
	{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
	{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = {
	0, 1, 2, 2, 3, 0
};

// Window const sizes
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// How many frames should be in flight at once
//	Two lets the CPU and GPU work on their own tasks at the same time,
//	but three or more frames in flight may let the CPU get ahead of the GPU,
//	causing latency frames. Extra latency isn't usually desired
const int MAX_FRAMES_IN_FLIGHT = 2;

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

// glfw resize callback function
static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->windowResized();
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

	// Create the GLFW window
	window_ = glfwCreateWindow(WIDTH, HEIGHT, "My Vulkan Window", nullptr, nullptr);

	// We want this if we need to explicitly handle a resize.
	// Make sure glfw knows the user pointer
	glfwSetWindowUserPointer(window_, this);
	glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
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

	// Create a layout for uniform variables
	createDescriptorSetLayout();

	// Create a graphics pipeline
	createGraphicsPipeline();

	// Create frame buffers
	createFramebuffers();

	// Create the command pool
	createCommandPool();

	// Create the vertex buffer
	createVertexBuffer();

	// Create the index buffer
	createIndexBuffer();

	// Create the uniform buffers
	createUniformBuffers();

	// Create a descriptor pool
	createDescriptorPool();

	// Create descriptor sets
	createDescriptorSets();

	// Create the command buffers
	createCommandBuffers();

	// Create the sync objects
	createSyncObjects();
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
		drawFrame();
	}

	// This will free up the semaphores and fences
	vkDeviceWaitIdle(logicalDevice_);
}

// Cleanup function
void HelloTriangleApplication::cleanup() {
	// Note: The VkPhysicalDevice is destroyed when the instance is destroyed
	//	so we don't need to worry about it

	// Destroy the swap chain
	cleanupSwapChain();

	// Destroy the uniform buffers
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		// Destroying the buffer first makes it so that the memory is not in use
		//	, allowing it to be freed with no issues
		vkDestroyBuffer(logicalDevice_, uniformBuffers_[i], nullptr);
		vkFreeMemory(logicalDevice_, uniformBuffersMemory_[i], nullptr);
	}

	// Destroy the descriptor pool, which will implicitly destroy any allocated sets
	vkDestroyDescriptorPool(logicalDevice_, descriptorPool_, nullptr);
	
	// Destroy the uniform layout descriptor
	vkDestroyDescriptorSetLayout(logicalDevice_, descriptorSetLayout_, nullptr);

	// Destroy the index buffer and free its memory
	vkDestroyBuffer(logicalDevice_, indexBuffer_, nullptr);
	vkFreeMemory(logicalDevice_, indexBufferMemory_, nullptr);

	// Destroy the vertex buffer
	vkDestroyBuffer(logicalDevice_, vertexBuffer_, nullptr);

	// After the vertex buffer is destroyed, we can then free the memory attached to it
	vkFreeMemory(logicalDevice_, vertexBufferMemory_, nullptr);

	// Destroy the graphics pipeline
	vkDestroyPipeline(logicalDevice_, gfxPipeline_, nullptr);

	// Destroy the pipeline layout
	vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);

	// Destroy the render passs
	vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);

	// Destroy the sync objects we have
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		vkDestroySemaphore(logicalDevice_, renderFinishedSemaphores_[i], nullptr);
		vkDestroySemaphore(logicalDevice_, imageAvailableSemaphores_[i], nullptr);
		vkDestroyFence(logicalDevice_, inFlightFences_[i], nullptr);
	}

	// Don't forget to destroy the command pool
	vkDestroyCommandPool(logicalDevice_, commandPool_, nullptr);

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

	// Getting binding and attribute descriptions for this pipeline
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	// Spacing between data and whether data is per-vertex or per-instance (instancing)
	// Optional
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	// Type of attributes passed to vertex shader, what binding they associate to, and at which offset.
	// Optional
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

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
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

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

	// We are specifying a uniform layout
	pipelineLayoutInfo.setLayoutCount = 1; // Optional
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_; // Optional
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

	//*****************************************************************************
	//	Subpass dependencies
	//		- Subpasses in a render pass take care of image layout transitions,
	//			these transitions controlled by subpass dependencies
	//		- Specified memory and execution dependencies between subpasses
	//		- Doing this because we need to wait to acquire the image before we
	//			render
	//*****************************************************************************
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Implicit subpass before the render pass
	dependency.dstSubpass = 0; // This is our subpass

	// We wait for our swap chain to finish reading from the color attachment
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	// Wait until the color attachment is fully written to
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	// Create the render pass
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
void HelloTriangleApplication::createCommandBuffers() {
	// Resize the command buffer vector to how many frames we want to handle
	commandBuffers_.resize(MAX_FRAMES_IN_FLIGHT);

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
	allocInfo.commandBufferCount = (uint32_t)commandBuffers_.size();

	// Now actually create the command buffer
	if (vkAllocateCommandBuffers(logicalDevice_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
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

	// Specify the vertex buffer we want to use for rendering
	VkBuffer vertexBuffers[] = { vertexBuffer_ };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

	// Also specify the index buffer we are using
	vkCmdBindIndexBuffer(commandBuffer, indexBuffer_, 0, VK_INDEX_TYPE_UINT16);

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

	// Make sure to bind the uniforms correctly here
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout_, 0, 1, &descriptorSets_[curFrame_], 0, nullptr);

	// Now with those set, we can call the command to draw our triangles
	// Parameters:
	//	The command buffer to record to
	//	How many vertices to render
	//	How many instances to render
	//	Offset into vertex buffer, defines lowest value of gl_VertexIndex
	//	Offset for instanced rendering, defined lowest value of gl_InstanceIndex
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	// Now we end the render pass
	vkCmdEndRenderPass(commandBuffer);

	// Finish rerecording the command buffer
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to record command buffer!");
	}
}

//*****************************************************************************
//	Actually rendering a frame. The steps in Vulkan:
//		- Wait for previous frame to finish
//		- acquire an image from the swap chain
//		- record a command buffer which draws the scene onto that image
//		- submit the recorded command buffer
//		- present the swap chain image
// 
//	Because we need to be careful of synchronization problems, we will have
//		Semaphores and Fences
// 
//		- Semaphores are used for swapchain operations because they are on GPU
//		  and CPU (host) doesn't need to wait around for them
//		- Fences are used for waiting on the previous frame to finish, the CPU
//		  (host) needs to wait so we aren't drawing more than one frame at a
//		  time.
//*****************************************************************************
void HelloTriangleApplication::drawFrame() {
	// Wait for the previous frame to finish
	// Parameters:
	//	Logical Device
	//	How many fences
	//	Pointer to all the fences
	//	Wait for all fences?
	//	Timeout
	vkWaitForFences(logicalDevice_, 1, &inFlightFences_[curFrame_], VK_TRUE, UINT64_MAX);

	// Now we grab an image from our swap chain
	// Returns an index of an image in our swap chain, as well as uses the semaphore for acquiring
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice_, swapChain_, UINT64_MAX, imageAvailableSemaphores_[curFrame_], VK_NULL_HANDLE, &imageIndex);

	// We need to handle if the current swap chain is invalid
	//	ERROR_OUT_OF_DATE_KHR - 
	//		Swap chain is incompatible with surface and can't be used for rendering. Usually window resize
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		framebufferResized_ = false;
		recreateSwapChain();
		return;
	}
	//	SUBOPTIMAL_KHR -
	//		Swap chain can still be used, but surface properties don't match exactly
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	// After waiting for the fence, remember to reset the fence to unsignaled
	//	Only reset when we guarantee work is being submitted
	vkResetFences(logicalDevice_, 1, &inFlightFences_[curFrame_]);

	// Reset our command buffer, then record our command buffer
	vkResetCommandBuffer(commandBuffers_[curFrame_], 0);
	recordCommandBuffer(commandBuffers_[curFrame_], imageIndex);

	// Update the uniform buffer
	updateUniformBuffer(curFrame_);

	// Now we want to submit our command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// Wait on these semaphores before executing commands
	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores_[curFrame_] };

	// Wait on writing to the color attachment until semaphore is available
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	// What command buffers are being submitted
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers_[curFrame_];

	// What semaphores to signal once the command buffers submitted finish execution
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores_[curFrame_] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Submit the command buffer to the queue
	//	Passing inFlightFence will signal it once the commands finish
	if (vkQueueSubmit(graphicsQueue_, 1, &submitInfo, inFlightFences_[curFrame_]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}
	
	//*****************************************************************************
	//	Presentation
	//		Frame should now be rendered, we just need to present it
	//*****************************************************************************
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// What semaphores to wait on before we can present
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	// What swap chains to present images to, as well as the index for each swap chain
	VkSwapchainKHR swapChains[] = { swapChain_ };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	// Allows specification of a VkResult array to check for each swap chain if presentation was successful
	presentInfo.pResults = nullptr; // Optional

	// Now present the image!
	result = vkQueuePresentKHR(presentQueue_, &presentInfo);

	// Recreate swap chain if out of date or suboptimal
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized_) {
		framebufferResized_ = false;
		recreateSwapChain();
	}

	// This is just a failure
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}

	// Remember to advance to the next frame
	curFrame_ = (curFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

//*****************************************************************************
//	Function for creating the semaphores and fences we need
//*****************************************************************************
void HelloTriangleApplication::createSyncObjects() {
	// Reallocate the vectors for the sync objects
	imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

	// Creation structures for semaphores and fences
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Fence starts signaled

	// Create all the semaphores and fences we need
	/*
	if (vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &imageAvailableSemaphore_) != VK_SUCCESS ||
		vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &renderFinishedSemaphore_) != VK_SUCCESS ||
		vkCreateFence(logicalDevice_, &fenceInfo, nullptr, &inFlightFence_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Semaphores or Fences");
	}
	*/
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		if (vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
			vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS ||
			vkCreateFence(logicalDevice_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Semaphores or Fences");
		}
	}
}

void HelloTriangleApplication::cleanupSwapChain() {
	// Destroy the frame buffers
	for (auto framebuffer : swapChainFramebuffers_) {
		vkDestroyFramebuffer(logicalDevice_, framebuffer, nullptr);
	}

	// Clean up the image views
	for (auto imageView : swapChainImageViews_) {
		vkDestroyImageView(logicalDevice_, imageView, nullptr);
	}

	// Destroy the swapchain
	vkDestroySwapchainKHR(logicalDevice_, swapChain_, nullptr);
}

//*****************************************************************************
//	This function is for recreating the swap chain to handle the window being
//		resized
//*****************************************************************************
void HelloTriangleApplication::recreateSwapChain() {
	// Need to handle if the window is minimized
	int width = 0, height = 0;
	glfwGetFramebufferSize(window_, &width, &height);
	while (width == 0 || height == 0) {
		// Always call glfwGetFrameBuffer size
		glfwGetFramebufferSize(window_, &width, &height);

		// glfw will wait until an event is called
		glfwWaitEvents();
	}

	// Free up the locks
	vkDeviceWaitIdle(logicalDevice_);

	// Clean up the current swap chain
	cleanupSwapChain();

	// Re-Create the swap chain
	createSwapChain();
	createImageViews();
	createFramebuffers();
}

// Function for creating an actual vertex buffer
void HelloTriangleApplication::createVertexBuffer() {
	// Specify the buffer size
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// We will create a staging buffer, that will be temporary buffer on CPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Two types of transfer flags:
	//	SRC - Buffer can be used as a source in a memory transfer operation
	//	DST - Buffer can be used as a destination in a memory transfer operation
	VkBufferUsageFlags stagingBufferType = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	// This buffer is visible to the CPU
	VkMemoryPropertyFlags stagingMemoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(bufferSize, stagingBufferType, stagingMemoryProps, stagingBuffer, stagingBufferMemory);

	// Now we have data we want to copy into the buffer
	void* data;

	// Map the memory so CPU knows where to put memory
	vkMapMemory(logicalDevice_, stagingBufferMemory, 0, bufferSize, 0, &data);

	// Simply memcpy our vertex data into the buffer
	memcpy(data, vertices.data(), (size_t)bufferSize);

	// Now unmap the memory, since we have already copied the data over
	vkUnmapMemory(logicalDevice_, stagingBufferMemory);

	// Now create the vertex buffer (It can now be the destination for a memory transfer
	VkBufferUsageFlags bufferType = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	// This buffer only exists on the GPU
	VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// Now, when we create the vertex buffer, we can copy the staging buffer info the vertex buffer
	createBuffer(bufferSize, bufferType, memoryProps, vertexBuffer_, vertexBufferMemory_);
	copyBuffer(stagingBuffer, vertexBuffer_, bufferSize);

	// Once the data is copied over, we should free up the temporary staging buffer
	vkDestroyBuffer(logicalDevice_, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice_, stagingBufferMemory, nullptr);

	// NOTES:
	// Driver may not immediately copy data into the buffer memory (due to caching as example)
	//	Two ways to solve this are:
	//	  - Use memory heap that is host coherent (which we already indicated)
	//	  - Manually flush the memory after writing and invalidating the memory before reading
	//	The first way may have worse performance

	// In a real world application, you should not call VkAllocateMemory for every individual buffer
	//	Maximum number of simultaneous memory allocations is limited by the physical device,
	//	which can be as low as 4096, meaning only 4096 buffers if allocated this way.
}

void HelloTriangleApplication::createIndexBuffer() {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// We will create a staging buffer, that will be temporary buffer on CPU
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// Two types of transfer flags:
	//	SRC - Buffer can be used as a source in a memory transfer operation
	//	DST - Buffer can be used as a destination in a memory transfer operation
	VkBufferUsageFlags stagingBufferType = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	// This buffer is visible to the CPU
	VkMemoryPropertyFlags stagingMemoryProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	createBuffer(bufferSize, stagingBufferType, stagingMemoryProps, stagingBuffer, stagingBufferMemory);

	// Now we have data we want to copy into the buffer
	void* data;

	// Map the memory so CPU knows where to put memory
	vkMapMemory(logicalDevice_, stagingBufferMemory, 0, bufferSize, 0, &data);

	// Simply memcpy our vertex data into the buffer
	memcpy(data, indices.data(), (size_t)bufferSize);

	// Now unmap the memory, since we have already copied the data over
	vkUnmapMemory(logicalDevice_, stagingBufferMemory);

	// Now create the index buffer (It can now be the destination for a memory transfer
	VkBufferUsageFlags bufferType = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

	// This buffer only exists on the GPU
	VkMemoryPropertyFlags memoryProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	
	createBuffer(bufferSize, bufferType, memoryProps, indexBuffer_, indexBufferMemory_);

	// copy the temp buffer over to the GPU
	copyBuffer(stagingBuffer, indexBuffer_, bufferSize);

	// Delete the temporary staging buffer
	vkDestroyBuffer(logicalDevice_, stagingBuffer, nullptr);
	vkFreeMemory(logicalDevice_, stagingBufferMemory, nullptr);
}

// We need to figure out what types of memory our GPU has
uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	// First, figure out what available types of memory we have
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_, &memProperties);

	// Above struct has two different arrays:
	//	memoryTypes
	//	memoryHeaps - Dedicated memory resources like VRAM and swap space in RAM
	
	// For the moment, we only care about the memory type and not the heap, but it can affect performance
	//	based on where the memory heap is
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		// Checking the bit flags for the memory type we want
		// We also need to check that the memory can be written to from the CPU
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

// Abstract function for creating a buffer
void HelloTriangleApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
	// The usual of creating a create struct
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

	// Size of the buffer in bytes
	bufferInfo.size = size;

	// How will this buffer be used. It is a vertex buffer
	bufferInfo.usage = usage;

	// Buffers can be owned by a specific queue family or be shared between multiple.
	// It will only be used for the graphics queue, so we are not worried
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// Actually create the buffer
	if (vkCreateBuffer(logicalDevice_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	// Get the requirements to allocate memory
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice_, buffer, &memRequirements);

	// Now we want to actually allocate memory to the buffer
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;

	// Make sure we specify the memory type we need
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(logicalDevice_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	// After successful memory allocation, we can now associate this memory with the vertex buffer
	//	Last parameter is an offset into the region of memory
	vkBindBufferMemory(logicalDevice_, buffer, bufferMemory, 0);
}

// Fucntion for copying over a buffer to another buffer
void HelloTriangleApplication::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {
	// We will create a temporary command buffer to perform the operation.
	//	It is possible to create a separate command pool for these operations, and it may
	//	apply allocation optimizations.
	//		When creating this command pool, use TRANSIENT_BIT, as the commands will be 
	//		short lived
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool_;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &commandBuffer);

	// Now immediately start recording the command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // We will only submit this command once
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	// Now submit the copy buffer command
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

	// Remember to end the command buffer
	vkEndCommandBuffer(commandBuffer);

	// Now we need to execute the command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE);

	// We just want to wait for this command to finish executing
	//	If we used a fence, we could allow multiple memory transfers in parallel
	vkQueueWaitIdle(graphicsQueue_);

	// We did everything we needed with out command buffer, remember to free it
	vkFreeCommandBuffers(logicalDevice_, commandPool_, 1, &commandBuffer);
}

// Function for creating a uniform variable layout for use in shaders
void HelloTriangleApplication::createDescriptorSetLayout() {
	// Struct for defining the binding layout
	VkDescriptorSetLayoutBinding uboLayoutBinding{};

	// Uniform buffer is laid out at 0
	uboLayoutBinding.binding = 0;

	// It is a uniform buffer
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	// There is one uniform block
	uboLayoutBinding.descriptorCount = 1;

	// What stage are the uniforms in
	//	Can be an or combination of multiple flags,
	//	or just ALL_GRAPHICS
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	// Only relevant for image sampling
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	// Now we will actually create the layout
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &uboLayoutBinding;

	if (vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void HelloTriangleApplication::createUniformBuffers() {
	// How big is the uniform buffer
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	// Resize our vectors
	uniformBuffers_.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMemory_.resize(MAX_FRAMES_IN_FLIGHT);
	uniformBuffersMapped_.resize(MAX_FRAMES_IN_FLIGHT);

	// Create the buffers
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		// Create a Uniform buffer and allocate memory to it
		VkBufferUsageFlags bufferUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		createBuffer(bufferSize, bufferUsage, memProps, uniformBuffers_[i], uniformBuffersMemory_[i]);

		// Map the memory to GPU
		vkMapMemory(logicalDevice_, uniformBuffersMemory_[i], 0, bufferSize, 0, &uniformBuffersMapped_[i]);
	}
}

// Helper function for updating the uniform buffer of a given image
void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage) {
	// Get what the start time is as a static variable
	static auto startTime = std::chrono::high_resolution_clock::now();

	// Figure out the total run time
	auto curTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(curTime - startTime).count();

	// Fill the Uniform Buffer struct
	UniformBufferObject ubo{};

	// Generate a rotation matrix that rotates around the z-axis
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Generate a view matrix given a camera eye, a position to look at, and an up vector (z is up in this case)
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Generate a perspective matrix to handle perspective. Use a 45 degree FOV, calculate aspect, and near and far planes
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent_.width / (float)swapChainExtent_.height, 0.1f, 10.0f);

	// Need to multiply by -1 because we are in Vulkan, as glm was designed for OpenGL
	ubo.proj[1][1] *= -1;

	// Now we have defined all the matrices, now we want to mem copy them into our buffer
	memcpy(uniformBuffersMapped_[currentImage], &ubo, sizeof(ubo));
}

// In order to tell the shader the uniform information, so we need to
//	specify descriptor sets
void HelloTriangleApplication::createDescriptorPool() {
	// Define a pool size, which needs to know:
	//	What the pool will be used for
	//	How many descriptors there are
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	// Creation struct for a descriptor pool
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;

	// What is the maximum number of descriptors that can be allocated
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	// Create the pool
	if (vkCreateDescriptorPool(logicalDevice_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

// Function for creating descriptor sets for uniform buffers
void HelloTriangleApplication::createDescriptorSets() {
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout_);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	// What pool descriptors will be allocated from
	allocInfo.descriptorPool = descriptorPool_;

	// How many descriptor sets will be allocated
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	// pointer to the layouts
	allocInfo.pSetLayouts = layouts.data();

	// Allocate the descriptor sets
	descriptorSets_.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(logicalDevice_, &allocInfo, descriptorSets_.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets");
	}

	// Now that the descriptors have been allocated, they need to be configured
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		// Specify the uniform buffer info
		VkDescriptorBufferInfo bufferInfo{};

		// What buffer this descriptor has
		bufferInfo.buffer = uniformBuffers_[i];

		// Binding offset is zero
		bufferInfo.offset = 0;

		// Specify a size that will be written to
		bufferInfo.range = sizeof(UniformBufferObject);

		// Now we need to write the descriptors
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets_[i];
		descriptorWrite.dstBinding = 0; // Uniform binding is 0
		descriptorWrite.dstArrayElement = 0; // No array, so 0

		// Descriptor is means for a uniform buffer
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;

		// used for buffer descriptors
		descriptorWrite.pBufferInfo = &bufferInfo;

		// used for image descriptors
		descriptorWrite.pImageInfo = nullptr; // Optional

		// used for buffer view descriptors
		descriptorWrite.pTexelBufferView = nullptr; // Optional

		// Now update the descriptors
		vkUpdateDescriptorSets(logicalDevice_, 1, &descriptorWrite, 0, nullptr);
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
