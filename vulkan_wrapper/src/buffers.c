#include "engine.h"
#include <stdio.h>

void createVertexBuffer(Engine* engine) {
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

    uint32_t memoryTypeIndex = UINT32_MAX;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memRequirements.memoryTypeBits & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
            memoryTypeIndex = i;
            break;
        }
    }
    if (memoryTypeIndex == UINT32_MAX) {
        fprintf(stderr, "Failed to find suitable memory type for vertex buffer\n");
        vkDestroyBuffer(engine->device, engine->vertexBuffer, NULL);
        engine->vertexBuffer = VK_NULL_HANDLE;
        return;
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    if (vkAllocateMemory(engine->device, &allocInfo, NULL, &engine->vertexBufferMemory) != VK_SUCCESS) {
        fprintf(stderr, "Failed to allocate vertex buffer memory\n");
        vkDestroyBuffer(engine->device, engine->vertexBuffer, NULL);
        engine->vertexBuffer = VK_NULL_HANDLE;
        return;
    }

    if (vkBindBufferMemory(engine->device, engine->vertexBuffer, engine->vertexBufferMemory, 0) != VK_SUCCESS) {
        fprintf(stderr, "Failed to bind vertex buffer memory\n");
        vkFreeMemory(engine->device, engine->vertexBufferMemory, NULL);
        vkDestroyBuffer(engine->device, engine->vertexBuffer, NULL);
        engine->vertexBufferMemory = VK_NULL_HANDLE;
        engine->vertexBuffer = VK_NULL_HANDLE;
    }
}

void createUniformBuffer(Engine* engine) {
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

    if (vkBindBufferMemory(engine->device, engine->uniformBuffer, engine->uniformBufferMemory, 0) != VK_SUCCESS) {
        fprintf(stderr, "Failed to bind uniform buffer memory\n");
    }
}