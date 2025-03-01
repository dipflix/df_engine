import 'dart:ffi';

final class VkExtent2D extends Struct {
  @Uint32()
  external int width;
  @Uint32()
  external int height;
}