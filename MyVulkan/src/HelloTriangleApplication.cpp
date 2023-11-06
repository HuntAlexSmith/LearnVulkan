/**************************************************************************//**
*	@file   Stub.cpp
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		Implementation of the HelloTriangleApplication class
******************************************************************************/

#include "HelloTriangleApplication.h"

#include <stdexcept>
#include <iostream>

// Window const sizes
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// Vulkan validation layers
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

// Macros for if the validation layers are on or off
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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

	// Pick the physical device
	pickPhysicalDevice();
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

	// Destroy the debug messenger if validation layers are enabled
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(vkInstance_, debugMessenger_, nullptr);
	}

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

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device) {
	// Query the device properties
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	// Query the device features
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	// Check if the device is what we want (Discrete and has geometry shader)
	bool result = deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
	result = result && (deviceFeatures.geometryShader);

	return result;
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

	// Check all the devices and check if they are suitable
	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			vkPhysicalDevice_ = device;
			break;
		}
	}

	// If we didn't find a suitable device, throw
	if (vkPhysicalDevice_ == VK_NULL_HANDLE) {
		throw std::runtime_error("Failed to find suitable GPU");
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
