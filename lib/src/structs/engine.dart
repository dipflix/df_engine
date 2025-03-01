import 'dart:ffi';

import 'vk_extend_2d.dart';

final class Engine extends Struct {
  external Pointer window;
  external Pointer instance;
  external Pointer physicalDevice;
  external Pointer device;
  external Pointer graphicsQueue;
  external Pointer surface;
  external Pointer swapchain;
  external Pointer swapchainImages;
  @Uint32()
  external int swapchainImageCount;
  external Pointer swapchainImageViews;
  external VkExtent2D swapchainExtent;
  @Int32()
  external int swapchainImageFormat;
  external Pointer renderPass;
  external Pointer framebuffers;
  external Pointer commandPool;
  external Pointer commandBuffer;
  external Pointer imageAvailableSemaphore;
  external Pointer renderFinishedSemaphore;
  @Array(4)
  external Array<Float> clearColor;
}
