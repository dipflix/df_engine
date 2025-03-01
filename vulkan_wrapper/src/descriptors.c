#include "engine.h"
#include <stdio.h>

void createDescriptorSetLayout(Engine* engine) {
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

void createDescriptorPool(Engine* engine) {
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

void createDescriptorSet(Engine* engine) {
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