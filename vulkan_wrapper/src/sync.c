#include "engine.h"
#include <stdio.h>

void createSyncObjects(Engine* engine) {
    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };

    if (vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, &engine->imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(engine->device, &semaphoreInfo, NULL, &engine->renderFinishedSemaphore) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create semaphores\n");
        }
}