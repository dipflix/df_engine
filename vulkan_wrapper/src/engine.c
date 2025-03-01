#include "engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static uint32_t clamp(uint32_t value, uint32_t min, uint32_t max) {
    return (value < min) ? min : (value > max) ? max : value;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(VkSurfaceFormatKHR* formats, uint32_t formatCount) {
    for (uint32_t i = 0; i < formatCount; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }
    return formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(VkPresentModeKHR* modes, uint32_t modeCount) {
    for (uint32_t i = 0; i < modeCount; i++) {
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D chooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR* capabilities) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D extent = {
        .width = (uint32_t)width,
        .height = (uint32_t)height
    };

    extent.width = clamp(extent.width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    extent.height = clamp(extent.height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    return extent;
}

static VkShaderModule createShaderModule(VkDevice device, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open shader file: %s\n", filename);
        return VK_NULL_HANDLE;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(fileSize);
    fread(buffer, 1, fileSize, file);
    fclose(file);

    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fileSize,
        .pCode = (uint32_t*)buffer
    };

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create shader module from %s\n", filename);
        shaderModule = VK_NULL_HANDLE;
    }

    free(buffer);
    return shaderModule;
}

static void createSwapChain(Engine* engine) {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(engine->physicalDevice, engine->surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physicalDevice, engine->surface, &formatCount, NULL);
    VkSurfaceFormatKHR* formats = (VkSurfaceFormatKHR*)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(engine->physicalDevice, engine->surface, &formatCount, formats);

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physicalDevice, engine->surface, &presentModeCount, NULL);
    VkPresentModeKHR* presentModes = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(engine->physicalDevice, engine->surface, &presentModeCount, presentModes);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(formats, formatCount);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModes, presentModeCount);
    VkExtent2D extent = chooseSwapExtent(engine->window, &capabilities);

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = engine->surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    if (vkCreateSwapchainKHR(engine->device, &createInfo, NULL, &engine->swapchain) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create swap chain\n");
    }

    vkGetSwapchainImagesKHR(engine->device, engine->swapchain, &engine->swapchainImageCount, NULL);
    engine->swapchainImages = (VkImage*)malloc(engine->swapchainImageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(engine->device, engine->swapchain, &engine->swapchainImageCount, engine->swapchainImages);

    engine->swapchainImageFormat = surfaceFormat.format;
    engine->swapchainExtent = extent;

    engine->swapchainImageViews = (VkImageView*)malloc(engine->swapchainImageCount * sizeof(VkImageView));
    for (uint32_t i = 0; i < engine->swapchainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = engine->swapchainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = engine->swapchainImageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1
        };

        if (vkCreateImageView(engine->device, &viewInfo, NULL, &engine->swapchainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create image view %d\n", i);
        }
    }

    free(formats);
    free(presentModes);
}

static void createRenderPass(Engine* engine) {
    VkAttachmentDescription colorAttachment = {
        .format = engine->swapchainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentRef = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentRef
    };

    VkRenderPassCreateInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        .subpassCount = 1,
        .pSubpasses = &subpass
    };

    if (vkCreateRenderPass(engine->device, &renderPassInfo, NULL, &engine->renderPass) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render pass\n");
    }
}

static void createFramebuffers(Engine* engine) {
    engine->framebuffers = (VkFramebuffer*)malloc(engine->swapchainImageCount * sizeof(VkFramebuffer));

    for (uint32_t i = 0; i < engine->swapchainImageCount; i++) {
        VkImageView attachments[] = {engine->swapchainImageViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = engine->renderPass,
            .attachmentCount = 1,
            .pAttachments = attachments,
            .width = engine->swapchainExtent.width,
            .height = engine->swapchainExtent.height,
            .layers = 1
        };

        if (vkCreateFramebuffer(engine->device, &framebufferInfo, NULL, &engine->framebuffers[i]) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create framebuffer %d\n", i);
        }
    }
}

static void createCommandPoolAndBuffers(Engine* engine) {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0,
        .flags = 0
    };

    if (vkCreateCommandPool(engine->device, &poolInfo, NULL, &engine->commandPool) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create command pool\n");
    }

    VkCommandBufferAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = engine->commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    if (vkAllocateCommandBuffers(engine->device, &allocInfo, &engine->commandBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate command buffers\n");
    }
}

static void createSyncObjects(Engine* engine) {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    if (vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, &engine->imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, &engine->renderFinishedSemaphore) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create semaphores\n");
    }
}

static void createVertexBuffer(Engine* engine) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(Vertex3D) * 1024,
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(engine->device, &bufferInfo, NULL, &engine->vertexBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create vertex buffer\n");
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(engine->device, engine->vertexBuffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(engine->physicalDevice, &memProperties);

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    if (vkAllocateMemory(engine->device, &allocInfo, NULL, &engine->vertexBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate vertex buffer memory\n");
        return;
    }

    vkBindBufferMemory(engine->device, engine->vertexBuffer, engine->vertexBufferMemory, 0);
}

static void createUniformBuffer(Engine* engine) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = sizeof(float) * 16,
        .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vkCreateBuffer(engine->device, &bufferInfo, NULL, &engine->uniformBuffer) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create uniform buffer\n");
        return;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(engine->device, engine->uniformBuffer, &memRequirements);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(engine->physicalDevice, &memProperties);

    uint32_t memoryTypeIndex = 0;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    if (vkAllocateMemory(engine->device, &allocInfo, NULL, &engine->uniformBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate uniform buffer memory\n");
        return;
    }

    vkBindBufferMemory(engine->device, engine->uniformBuffer, engine->uniformBufferMemory, 0);
}

static void createDescriptorSetLayout(Engine* engine) {
    VkDescriptorSetLayoutBinding uboLayoutBinding = {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = NULL
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding
    };

    if (vkCreateDescriptorSetLayout(engine->device, &layoutInfo, NULL, &engine->descriptorSetLayout) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor set layout\n");
    }
}

static void createDescriptorPool(Engine* engine) {
    VkDescriptorPoolSize poolSize = {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1
    };

    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize,
        .maxSets = 1
    };

    if (vkCreateDescriptorPool(engine->device, &poolInfo, NULL, &engine->descriptorPool) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor pool\n");
    }
}

static void createDescriptorSet(Engine* engine) {
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = engine->descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &engine->descriptorSetLayout
    };

    if (vkAllocateDescriptorSets(engine->device, &allocInfo, &engine->descriptorSet) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate descriptor sets\n");
        return;
    }

    VkDescriptorBufferInfo bufferInfo = {
        .buffer = engine->uniformBuffer,
        .offset = 0,
        .range = sizeof(float) * 16
    };

    VkWriteDescriptorSet descriptorWrite = {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = engine->descriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .pBufferInfo = &bufferInfo
    };

    vkUpdateDescriptorSets(engine->device, 1, &descriptorWrite, 0, NULL);
}

static void createGraphicsPipeline(Engine* engine) {
    VkShaderModule vertShaderModule = createShaderModule(engine->device, ".shaders/vertex3d.spv");
    VkShaderModule fragShaderModule = createShaderModule(engine->device, ".shaders/fragment3d.spv");

    if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
        fprintf(stderr, "Failed to load shaders\n");
        return;
    }

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertShaderModule,
            .pName = "main"
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragShaderModule,
            .pName = "main"
        }
    };

    VkVertexInputBindingDescription bindingDescription = {
        .binding = 0,
        .stride = sizeof(Vertex3D),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkVertexInputAttributeDescription attributeDescriptions[] = {
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex3D, x) },
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex3D, r) }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = 2,
        .pVertexAttributeDescriptions = attributeDescriptions
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (float)engine->swapchainExtent.width,
        .height = (float)engine->swapchainExtent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = engine->swapchainExtent
    };

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor
    };

    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE
    };

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE
    };

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment
    };

    createDescriptorSetLayout(engine);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &engine->descriptorSetLayout
    };

    if (vkCreatePipelineLayout(engine->device, &pipelineLayoutInfo, NULL, &engine->pipelineLayout) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create pipeline layout\n");
        return;
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .layout = engine->pipelineLayout,
        .renderPass = engine->renderPass,
        .subpass = 0
    };

    if (vkCreateGraphicsPipelines(engine->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &engine->graphicsPipeline) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create graphics pipeline\n");
    }

    vkDestroyShaderModule(engine->device, fragShaderModule, NULL);
    vkDestroyShaderModule(engine->device, vertShaderModule, NULL);
}

EXPORT Engine* engine_create(int width, int height, const char* title) {
    Engine* engine = (Engine*)calloc(1, sizeof(Engine));

    engine->clearColor[0] = 0.0f;
    engine->clearColor[1] = 0.0f;
    engine->clearColor[2] = 1.0f;
    engine->clearColor[3] = 1.0f;

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        free(engine);
        return NULL;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    engine->window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!engine->window) {
        glfwTerminate();
        free(engine);
        return NULL;
    }

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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

    if (vkCreateInstance(&instanceInfo, NULL, &engine->instance) != VK_SUCCESS) {
        engine_destroy(engine);
        return NULL;
    }

    if (glfwCreateWindowSurface(engine->instance, engine->window, NULL, &engine->surface) != VK_SUCCESS) {
        engine_destroy(engine);
        return NULL;
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(engine->instance, &deviceCount, NULL);
    VkPhysicalDevice* devices = (VkPhysicalDevice*)malloc(deviceCount * sizeof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(engine->instance, &deviceCount, devices);
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

    if (vkCreateDevice(engine->physicalDevice, &deviceInfo, NULL, &engine->device) != VK_SUCCESS) {
        engine_destroy(engine);
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

        if (callback) {
            callback(deltaTime);
        }

        glfwPollEvents();

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(engine->device, engine->swapchain, UINT64_MAX,
                                                engine->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
        if (result != VK_SUCCESS) {
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

        vkQueuePresentKHR(engine->graphicsQueue, &presentInfo);
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