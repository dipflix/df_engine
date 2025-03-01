import 'dart:ffi';

import 'package:ffi/ffi.dart';

final class Vertex3D extends Struct {
  @Float()
  external double x;
  @Float()
  external double y;
  @Float()
  external double z;
  @Float()
  external double r;
  @Float()
  external double g;
  @Float()
  external double b;

  factory Vertex3D(double x, double y, double z, double r, double g, double b) {
    final vertex = malloc<Vertex3D>().ref;
    vertex.x = x;
    vertex.y = y;
    vertex.z = z;
    vertex.r = r;
    vertex.g = g;
    vertex.b = b;
    return vertex;
  }
}