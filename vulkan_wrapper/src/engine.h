#ifndef ENGINE_H
#define ENGINE_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

typedef void (*FrameCallback)(float deltaTime);

typedef struct {
    float x, y, z;  // 3D позиция
    float r, g, b;  // Цвет
} Vertex3D;

typedef struct {
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImage* swapchainImages;
    uint32_t swapchainImageCount;
    VkImageView* swapchainImageViews;
    VkExtent2D swapchainExtent;
    VkFormat swapchainImageFormat;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkFramebuffer* framebuffers;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    float clearColor[4];
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    uint32_t vertexCount;
    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
} Engine;

EXPORT Engine* engine_create(int width, int height, const char* title);
EXPORT void engine_destroy(Engine* engine);
EXPORT void engine_run(Engine* engine, FrameCallback callback);
EXPORT void engine_set_clear_color(Engine* engine, float r, float g, float b, float a);
EXPORT void engine_set_vertices(Engine* engine, Vertex3D* vertices, uint32_t vertexCount);
EXPORT void engine_set_view_matrix(Engine* engine, float* matrix);

#endif