/**************************************************************************//**
*	@file   Stub.h
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		HelloTriangleApplication that will handle all of the Vulkan and
*		GLFW calls for getting a triangle on screen
******************************************************************************/

#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

// Struct for Queue Families
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}
};

class HelloTriangleApplication {
public:

	// Member functions
	void run();

private:

	// Helper functions
	void initWindow();
	void initVulkan();
	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void setupDebugMessenger();
	void mainloop();
	void cleanup();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	bool isDeviceSuitable(VkPhysicalDevice device);
	void pickPhysicalDevice();
	int rateDeviceSuitability(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();

	// Debug callback function
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	// Private variables
	GLFWwindow* window_;    //!< The GLFW Window
	VkInstance vkInstance_; //!< The Vulkan Instance

	VkDebugUtilsMessengerEXT debugMessenger_; //!< Debug Messenger for debug callbacks

	VkPhysicalDevice vkPhysicalDevice_ = VK_NULL_HANDLE; //!< The physical device Vulkan will use to render

	VkDevice logicalDevice_; //!< The logical device for Vulkan

	VkQueue graphicsQueue_; //!< The queue for graphics commands
};
