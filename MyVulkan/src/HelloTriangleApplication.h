/**************************************************************************//**
*	@file   Stub.h
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		HelloTriangleApplication that will handle all of the Vulkan and
*		GLFW calls for getting a triangle on screen
* 
*		All code is referenced from: https://vulkan-tutorial.com/
******************************************************************************/

#pragma once

// Note: GLFW provides a create surface function for each platform
//  we will use their function, but understand the different ways to do it
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <optional>
#include <array>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Struct for handling a mesh vertex
struct Vertex {
	glm::vec2 pos;   //!< Position of this vertex
	glm::vec3 color; //!< Color at this vertex

	// Function for getting how to bind this input
	static VkVertexInputBindingDescription getBindingDescription() {
		// Struct for specifying the description
		VkVertexInputBindingDescription bindingDescription{};

		// Where the input will bind to
		bindingDescription.binding = 0;

		// The stride of the input
		bindingDescription.stride = sizeof(Vertex);

		// Two options for this:
		//	VERTEX - Move to next data entry after each vertex
		//	INSTANCE - Move to next data entry after each instance
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	// Function for getting attribute descriptions
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		// Describe the position attribute

		// What binding the per-vertex data comes from
		attributeDescriptions[0].binding = 0;

		// The location of the attribute in the shader
		attributeDescriptions[0].location = 0;

		// The data type of the attribute. Common typings are:
		//	float - VK_FORMAT_R32_SFLOAT
		//	vec2 - VK_FORMAT_R32G32_SFLOAT
		//	vec3 - VK_FORMAT_R32G32B32_SFLOAT
		//	vec4 - VK_FORMAT_R32G32B32A32_SFLOAT
		// Make sure to also match the typings correctly:
		//	ivec2 - VK_FORMAT_R32G32_SINT
		//	uvec4 - VK_FORMAT_R32G32B32A32_UINT
		//	double - VK_FORMAT_R64_SFLOAT
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;

		// Number of bytes since the start of the per-vertex data
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		// Describe the color attribute
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

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

// Struct for our uniform buffer
struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class HelloTriangleApplication {
public:

	// Member functions
	void run();
	
	// Set flag for window being resized
	void windowResized() { framebufferResized_ = true; }

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
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffers();
	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void drawFrame();
	void createSyncObjects();
	void recreateSwapChain();
	void cleanupSwapChain();
	void createVertexBuffer();
	void createIndexBuffer();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
	void createDescriptorSetLayout();
	void createUniformBuffers();
	void updateUniformBuffer(uint32_t currentImage);
	void createDescriptorPool();
	void createDescriptorSets();

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

	VkRenderPass renderPass_; //!< The render pass
	VkDescriptorSetLayout descriptorSetLayout_; //!< Layout for uniform variables
	VkPipelineLayout pipelineLayout_; //!< The pipeline layout (uniform variables)

	VkPipeline gfxPipeline_; //!< The actual rendering pipeline

	std::vector<VkFramebuffer> swapChainFramebuffers_; //!< The Frame buffers in the swap chain

	VkCommandPool commandPool_; //!< Pool for managing buffers and command buffers

	std::vector<VkCommandBuffer> commandBuffers_; //!< Command buffer for the command pool

	std::vector<VkSemaphore> imageAvailableSemaphores_; //!< Semaphore for checking if image is available
	std::vector<VkSemaphore> renderFinishedSemaphores_; //!< Semaphore for checking if rendering has finished

	std::vector<VkFence> inFlightFences_; //!< An image is currently being rendered

	bool framebufferResized_ = false; //!< For handling window resize explicitly

	uint32_t curFrame_ = 0; //!< The current frame to render

	VkBuffer vertexBuffer_; //!< Vertex buffer for triangle mesh
	VkDeviceMemory vertexBufferMemory_; //!< Memory for our vertex buffer

	VkBuffer indexBuffer_; //!< Index buffer for indexed rendering
	VkDeviceMemory indexBufferMemory_; //!< The buffer memory for the index buffer
	
	std::vector<VkBuffer> uniformBuffers_; //!< Handles to uniform buffers
	std::vector<VkDeviceMemory> uniformBuffersMemory_; //!< The memory of the uniform buffers
	std::vector<void*> uniformBuffersMapped_; //!< The uniform buffers mapped for writing

	VkDescriptorPool descriptorPool_; //!< Pool for allocating descriptors
	std::vector<VkDescriptorSet> descriptorSets_; //!< The descriptor sets
};
