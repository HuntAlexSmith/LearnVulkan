/**************************************************************************//**
*	@file   Stub.h
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		HelloTriangleApplication that will handle all of the Vulkan and
*		GLFW calls for getting a triangle on screen
******************************************************************************/

#pragma once

// Note: GLFW provides a create surface function for each platform
//  we will use their function, but understand the different ways to do it
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>

// Struct for Queue Families
struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

// Struct for Swap Chain details
struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
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
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	void pickPhysicalDevice();
	int rateDeviceSuitability(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
	void createSurface();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();

	// Debug callback function
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	VkSurfaceKHR surface_; //!< The rendering surface

	// Private variables
	GLFWwindow* window_;    //!< The GLFW Window
	VkInstance vkInstance_; //!< The Vulkan Instance

	VkDebugUtilsMessengerEXT debugMessenger_; //!< Debug Messenger for debug callbacks

	VkPhysicalDevice vkPhysicalDevice_ = VK_NULL_HANDLE; //!< The physical device Vulkan will use to render

	VkDevice logicalDevice_; //!< The logical device for Vulkan

	VkQueue graphicsQueue_; //!< The queue for graphics commands
	VkQueue presentQueue_;  //!< Presentation queue

	VkSwapchainKHR swapChain_; //!< Member variable for the swap chain

	std::vector<VkImage> swapChainImages_;         //!< Vector of the swap chain images
	std::vector<VkImageView> swapChainImageViews_; //!< Vector of the swap chain image views

	VkFormat swapChainImageFormat_; //!< Image format of the swap chain
	VkExtent2D swapChainExtent_;    //!< The extent of the swap chain
};
