#include "engine.h"
#include <stdio.h>

void createCommandPoolAndBuffers(Engine* engine) {
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