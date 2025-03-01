#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void recreateSwapChain(Engine* engine) {
    // Ждём завершения всех операций
    vkDeviceWaitIdle(engine->device);

    // Очищаем старые ресурсы
    for (uint32_t i = 0; i < engine->swapchainImageCount; i++) {
        vkDestroyFramebuffer(engine->device, engine->framebuffers[i], NULL);
        vkDestroyImageView(engine->device, engine->swapchainImageViews[i], NULL);
    }
    free(engine->framebuffers);
    free(engine->swapchainImageViews);
    free(engine->swapchainImages);
    vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);

    // Пересоздаём swapchain и связанные ресурсы
    createSwapChain(engine);
    createRenderPass(engine); // Можно оптимизировать, если render pass не зависит от размера
    createFramebuffers(engine);
    createCommandPoolAndBuffers(engine); // Пересоздаём command buffer
}

EXPORT Engine* engine_create(int width, int height, const char* title) {
    Engine* engine = (Engine*)calloc(1, sizeof(Engine));
    if (!engine) {
        fprintf(stderr, "Failed to allocate memory for Engine\n");
        return NULL;
    }

    engine->clearColor[0] = 0.0f;
    engine->clearColor[1] = 0.0f;
    engine->clearColor[2] = 1.0f;
    engine->clearColor[3] = 1.0f;

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        free(engine);
        return NULL;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    engine->window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!engine->window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        free(engine);
        return NULL;
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensions) {
        fprintf(stderr, "Failed to get GLFW required extensions\n");
        glfwDestroyWindow(engine->window);
        glfwTerminate();
        free(engine);
        return NULL;
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = title,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Game Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };

    VkInstanceCreateInfo instanceInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
        .enabledLayerCount = 0
    };

    VkResult result = vkCreateInstance(&instanceInfo, NULL, &engine->instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance: %d\n", result);
        glfwDestroyWindow(engine->window);
        glfwTerminate();
        free(engine);
        return NULL;
    }

    result = glfwCreateWindowSurface(engine->instance, engine->window, NULL, &engine->surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create window surface: %d\n", result);
        vkDestroyInstance(engine->instance, NULL);
        glfwDestroyWindow(engine->window);
        glfwTerminate();
        free(engine);
        return NULL;
    }

    uint32_t deviceCount = 0;
    result = vkEnumeratePhysicalDevices(engine->instance, &deviceCount, NULL);
    if (result != VK_SUCCESS || deviceCount == 0) {
        fprintf(stderr, "Failed to enumerate physical devices: %d, count=%d\n", result, deviceCount);
        vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
        vkDestroyInstance(engine->instance, NULL);
        glfwDestroyWindow(engine->window);
        glfwTerminate();
        free(engine);
        return NULL;
    }

    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
    result = vkEnumeratePhysicalDevices(engine->instance, &deviceCount, devices);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to get physical devices: %d\n", result);
        free(devices);
        vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
        vkDestroyInstance(engine->instance, NULL);
        glfwDestroyWindow(engine->window);
        glfwTerminate();
        free(engine);
        return NULL;
    }
    engine->physicalDevice = devices[0];
    free(devices);

    VkDeviceQueueCreateInfo queueInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = &(float){1.0f}
    };

    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions
    };

    result = vkCreateDevice(engine->physicalDevice, &deviceInfo, NULL, &engine->device);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan device: %d\n", result);
        vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
        vkDestroyInstance(engine->instance, NULL);
        glfwDestroyWindow(engine->window);
        glfwTerminate();
        free(engine);
        return NULL;
    }

    vkGetDeviceQueue(engine->device, 0, 0, &engine->graphicsQueue);
    createSwapChain(engine);
    createRenderPass(engine);
    createFramebuffers(engine);
    createCommandPoolAndBuffers(engine);
    createSyncObjects(engine);
    createVertexBuffer(engine);
    createUniformBuffer(engine);
    createGraphicsPipeline(engine);
    createDescriptorPool(engine);
    createDescriptorSet(engine);

    return engine;
}

EXPORT void engine_destroy(Engine* engine) {
    if (engine->descriptorPool) vkDestroyDescriptorPool(engine->device, engine->descriptorPool, NULL);
    if (engine->descriptorSetLayout) vkDestroyDescriptorSetLayout(engine->device, engine->descriptorSetLayout, NULL);
    if (engine->uniformBufferMemory) vkFreeMemory(engine->device, engine->uniformBufferMemory, NULL);
    if (engine->uniformBuffer) vkDestroyBuffer(engine->device, engine->uniformBuffer, NULL);
    if (engine->vertexBufferMemory) vkFreeMemory(engine->device, engine->vertexBufferMemory, NULL);
    if (engine->vertexBuffer) vkDestroyBuffer(engine->device, engine->vertexBuffer, NULL);
    if (engine->graphicsPipeline) vkDestroyPipeline(engine->device, engine->graphicsPipeline, NULL);
    if (engine->pipelineLayout) vkDestroyPipelineLayout(engine->device, engine->pipelineLayout, NULL);
    if (engine->imageAvailableSemaphore) vkDestroySemaphore(engine->device, engine->imageAvailableSemaphore, NULL);
    if (engine->renderFinishedSemaphore) vkDestroySemaphore(engine->device, engine->renderFinishedSemaphore, NULL);
    if (engine->commandPool) vkDestroyCommandPool(engine->device, engine->commandPool, NULL);
    if (engine->framebuffers) {
        for (uint32_t i = 0; i < engine->swapchainImageCount; i++) {
            vkDestroyFramebuffer(engine->device, engine->framebuffers[i], NULL);
        }
        free(engine->framebuffers);
    }
    if (engine->renderPass) vkDestroyRenderPass(engine->device, engine->renderPass, NULL);
    if (engine->swapchainImageViews) {
        for (uint32_t i = 0; i < engine->swapchainImageCount; i++) {
            vkDestroyImageView(engine->device, engine->swapchainImageViews[i], NULL);
        }
        free(engine->swapchainImageViews);
    }
    if (engine->swapchainImages) free(engine->swapchainImages);
    if (engine->swapchain) vkDestroySwapchainKHR(engine->device, engine->swapchain, NULL);
    if (engine->device) vkDestroyDevice(engine->device, NULL);
    if (engine->surface) vkDestroySurfaceKHR(engine->instance, engine->surface, NULL);
    if (engine->instance) vkDestroyInstance(engine->instance, NULL);
    if (engine->window) glfwDestroyWindow(engine->window);
    glfwTerminate();
    free(engine);
}

EXPORT void engine_run(Engine* engine, FrameCallback callback) {
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(engine->window)) {
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;

        if (callback) callback(deltaTime);

        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(engine->window, &width, &height);
        if (width <= 0 || height <= 0) continue; // Игнорируем нулевые размеры

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(engine->device, engine->swapchain, UINT64_MAX,
                                                engine->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain(engine);
            continue;
        } else if (result != VK_SUCCESS) {
            fprintf(stderr, "Failed to acquire next image: %d\n", result);
            continue;
        }

        VkCommandBufferBeginInfo beginInfo = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
        };

        if (vkBeginCommandBuffer(engine->commandBuffer, &beginInfo) != VK_SUCCESS) {
            fprintf(stderr, "Failed to begin command buffer\n");
            continue;
        }

        VkClearValue clearColor = {{{engine->clearColor[0], engine->clearColor[1], engine->clearColor[2], engine->clearColor[3]}}};
        VkRenderPassBeginInfo renderPassInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = engine->renderPass,
            .framebuffer = engine->framebuffers[imageIndex],
            .renderArea.offset = {0, 0},
            .renderArea.extent = engine->swapchainExtent,
            .clearValueCount = 1,
            .pClearValues = &clearColor
        };

        vkCmdBeginRenderPass(engine->commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        if (engine->vertexCount > 0 && engine->graphicsPipeline != VK_NULL_HANDLE) {
            vkCmdBindPipeline(engine->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, engine->graphicsPipeline);
            VkBuffer vertexBuffers[] = {engine->vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(engine->commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindDescriptorSets(engine->commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    engine->pipelineLayout, 0, 1, &engine->descriptorSet, 0, NULL);
            vkCmdDraw(engine->commandBuffer, engine->vertexCount, 1, 0, 0);
        }

        vkCmdEndRenderPass(engine->commandBuffer);

        if (vkEndCommandBuffer(engine->commandBuffer) != VK_SUCCESS) {
            fprintf(stderr, "Failed to end command buffer\n");
            continue;
        }

        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &engine->imageAvailableSemaphore,
            .pWaitDstStageMask = (VkPipelineStageFlags[]){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
            .commandBufferCount = 1,
            .pCommandBuffers = &engine->commandBuffer,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &engine->renderFinishedSemaphore
        };

        if (vkQueueSubmit(engine->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
            fprintf(stderr, "Failed to submit queue\n");
            continue;
        }

        VkPresentInfoKHR presentInfo = {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &engine->renderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &engine->swapchain,
            .pImageIndices = &imageIndex
        };

        result = vkQueuePresentKHR(engine->graphicsQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain(engine);
            continue;
        } else if (result != VK_SUCCESS) {
            fprintf(stderr, "Failed to present queue: %d\n", result);
        }
        vkQueueWaitIdle(engine->graphicsQueue);
    }
}

EXPORT void engine_set_clear_color(Engine* engine, float r, float g, float b, float a) {
    engine->clearColor[0] = r;
    engine->clearColor[1] = g;
    engine->clearColor[2] = b;
    engine->clearColor[3] = a;
}

EXPORT void engine_set_vertices(Engine* engine, Vertex3D* vertices, uint32_t vertexCount) {
    if (vertexCount > 1024) {
        fprintf(stderr, "Too many vertices, max is 1024\n");
        return;
    }

    if (engine->vertexBuffer == VK_NULL_HANDLE || engine->vertexBufferMemory == VK_NULL_HANDLE) {
        fprintf(stderr, "Vertex buffer not initialized\n");
        return;
    }

    void* data;
    VkResult result = vkMapMemory(engine->device, engine->vertexBufferMemory, 0, sizeof(Vertex3D) * vertexCount, 0, &data);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to map vertex buffer memory: %d\n", result);
        return;
    }

    memcpy(data, vertices, sizeof(Vertex3D) * vertexCount);
    vkUnmapMemory(engine->device, engine->vertexBufferMemory);
    engine->vertexCount = vertexCount;
}

EXPORT void engine_set_view_matrix(Engine* engine, float* matrix) {
    if (engine->uniformBufferMemory == VK_NULL_HANDLE) {
        fprintf(stderr, "Uniform buffer not initialized\n");
        return;
    }

    void* data;
    VkResult result = vkMapMemory(engine->device, engine->uniformBufferMemory, 0, sizeof(float) * 16, 0, &data);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to map uniform buffer memory: %d\n", result);
        return;
    }

    memcpy(data, matrix, sizeof(float) * 16);
    vkUnmapMemory(engine->device, engine->uniformBufferMemory);
}