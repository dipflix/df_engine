#include "engine.h"
#include <stdio.h>
#include <stdlib.h>

void createFramebuffers(Engine* engine) {
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