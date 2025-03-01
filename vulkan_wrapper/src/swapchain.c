#include "engine.h"
#include <stdio.h>
#include <stdlib.h>

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

void createSwapChain(Engine* engine) {
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